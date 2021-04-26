// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/iostream_error.h"
#include "common/include/logging.h"
#include "common/include/posix_error.h"
#include "lib/include/platform/system_information.h"
#include "lib/include/system_details.h"

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

#include <sys/utsname.h>

namespace fs = std::filesystem;
using fmt::format;
using namespace std;

namespace {

optional<mmotd::system::KernelDetails> GetKernelDetails() {
    auto buf = utsname{};
    auto retval = uname(&buf);
    if (retval != 0) {
        auto error_str = mmotd::error::posix_error::to_string();
        LOG_ERROR("error calling uname, {}", error_str);
        return nullopt;
    }

    auto sys_name = string(buf.sysname);
    auto node_name = string(buf.nodename);
    auto release = string(buf.release);
    auto version = string(buf.version);
    auto machine = string(buf.machine);
    auto domain_name = string(buf.domainname);

    LOG_DEBUG("sys name      (kernel) : {}", sys_name);
    LOG_DEBUG("node name       (host) : {}", node_name);
    LOG_DEBUG("release      (release) : {}", release);
    LOG_DEBUG("version      (version) : {}", version);
    LOG_DEBUG("machine (architecture) : {}", machine);
    LOG_DEBUG("domain name            : {}", domain_name);

    auto kernel_details = mmotd::system::KernelDetails::from_string(sys_name, node_name, release, version, machine);
    LOG_DEBUG("host name              : {}", kernel_details.host_name);
    LOG_DEBUG("kernel version         : {}", kernel_details.kernel_version.version);
    LOG_DEBUG("kernel release:        : {}", kernel_details.kernel_version.release.to_string());
    LOG_DEBUG("kernel type            : {}", mmotd::system::to_string(kernel_details.kernel));
    LOG_DEBUG("architecture           : {}", mmotd::system::to_string(kernel_details.architecture));
    LOG_DEBUG("endian type            : {}", mmotd::system::to_string(kernel_details.endian));

    return make_optional(kernel_details);
}

constexpr static const char *OS_RELEASE = "/etc/os-release";

vector<string> GetOsVersionFile() {
    auto os_release_path = fs::path(OS_RELEASE);
    if (!fs::is_regular_file(os_release_path) || !fs::is_symlink(os_release_path)) {
        LOG_ERROR("{} release file does not exist", OS_RELEASE);
        return vector<string>{};
    }

    auto ifs = ifstream{};
    ifs.exceptions(std::ifstream::goodbit);
    ifs.open(os_release_path, ios_base::in);
    if (!ifs.is_open() || ifs.fail() || ifs.bad()) {
        auto ifs_err = mmotd::error::ios_flags::to_string(ifs);
        LOG_ERROR("unable to open {}, {}", OS_RELEASE, ifs_err);
        return vector<string>{};
    }

    auto version_lines = vector<string>{};
    for (string version_line; getline(ifs, version_line);) {
        if (ifs.fail() || ifs.bad()) {
            auto ifs_err = mmotd::error::ios_flags::to_string(ifs);
            LOG_ERROR("error reading {}, {}", OS_RELEASE, ifs_err);
            return vector<string>{};
        }
        version_lines.push_back(version_line);
    }

    return version_lines;
}

optional<int> ParseIndividualOsVersion(const string &version_str) {
    auto digit_classifier = boost::is_digit();

    auto updated_version_str = version_str;
    auto i = find_if(begin(updated_version_str), end(updated_version_str), [&digit_classifier](auto version_char) {
        return digit_classifier(version_char);
    });

    if (i != end(updated_version_str)) {
        auto offset = std::distance(begin(updated_version_str), i);
        LOG_DEBUG("found integer offset for {} to be {}", updated_version_str, offset);
        updated_version_str = updated_version_str.substr(offset);
    }
    if (updated_version_str.empty()) {
        LOG_DEBUG("stripped front of version string {} and now it's empty", version_str);
        return nullopt;
    }

    auto j = find_if(begin(updated_version_str), end(updated_version_str), [&digit_classifier](auto version_char) {
        return !digit_classifier(version_char);
    });

    if (j != end(updated_version_str)) {
        auto offset = std::distance(begin(updated_version_str), j);
        LOG_DEBUG("found integer offset for {} to be {}", updated_version_str, offset);
        updated_version_str = updated_version_str.substr(0, offset);
    }
    if (updated_version_str.empty()) {
        LOG_DEBUG("stripped back of version string {} and now it's empty", version_str);
        return nullopt;
    }

    LOG_DEBUG("stripped version string is {}", updated_version_str);

    return make_optional(std::stoi(updated_version_str));
}

optional<tuple<int, int, int>> ParseOsVersion(const string &version_str) {
    auto version_numbers = vector<string>{};
    boost::split(version_numbers, version_str, boost::is_any_of("."), boost::token_compress_on);
    if (version_numbers.size() < 2) {
        LOG_ERROR("unable to split '{}' into a version string", version_str);
        return nullopt;
    }
    int major, minor, patch = 0;
    auto major_holder = ParseIndividualOsVersion(version_numbers[0]);
    if (major_holder) {
        major = *major_holder;
    }
    auto minor_holder = ParseIndividualOsVersion(version_numbers[1]);
    if (minor_holder) {
        minor = *minor_holder;
    }
    if (version_numbers.size() > 2) {
        auto patch_holder = ParseIndividualOsVersion(version_numbers[2]);
        if (patch_holder) {
            patch = *patch_holder;
        }
    }
    return make_optional(make_tuple(major, minor, patch));
}

string ParseCodename(const string &input) {
    auto open_paren = find(begin(input), end(input), '(');
    if (open_paren == end(input)) {
        return string{};
    }
    auto close_paren = find(open_paren, end(input), ')');
    if (close_paren == end(input)) {
        return string{};
    }
    return string{open_paren, close_paren + 1};
}

optional<tuple<string, int, int, int>> GetOsVersion() {
    // File: /etc/os-release
    // NAME="Ubuntu"
    // VERSION="20.04.1 LTS (Focal Fossa)"
    // ID=ubuntu
    // ID_LIKE=debian
    // PRETTY_NAME="Ubuntu 20.04.1 LTS"
    // VERSION_ID="20.04"
    // HOME_URL="https://www.ubuntu.com/"
    // SUPPORT_URL="https://help.ubuntu.com/"
    // BUG_REPORT_URL="https://bugs.launchpad.net/ubuntu/"
    // PRIVACY_POLICY_URL="https://www.ubuntu.com/legal/terms-and-policies/privacy-policy"
    // VERSION_CODENAME=focal
    // UBUNTU_CODENAME=focal

    // Output:
    // Welcome to Ubuntu 20.04.1 LTS focal (GNU/Linux 5.4.0-58-generic x86_64)
    // Welcome to macOS 10 Catalina 10.15.7.19H15 (Darwin 19.6.0 x86_64)
    auto name = string{};
    auto codename = string{};
    auto major = int{0};
    auto minor = int{0};
    auto patch = int{0};

    const auto &file_data = GetOsVersionFile();
    for (const auto &file_line : file_data) {
        if (auto i = boost::ifind_first(file_line, "pretty_name="); !i.empty() && i.begin() == file_line.begin()) {
            auto parsed_version = ParseOsVersion(string{i.end(), file_line.end()});
            if (!parsed_version) {
                return nullopt;
            }
            std::tie(major, minor, patch) = *parsed_version;
        }
        if (auto i = boost::ifind_first(file_line, "name="); !i.empty() && i.begin() == file_line.begin()) {
            name = boost::trim_copy_if(string{i.end(), file_line.end()}, boost::is_any_of("\""));
        }
        if (auto i = boost::ifind_first(file_line, "version="); !i.empty() && i.begin() == file_line.begin()) {
            codename = ParseCodename(string{i.end(), file_line.end()});
        }
    }
    return make_optional(make_tuple(format(FMT_STRING("{} {}"), name, codename), major, minor, patch));
}

} // namespace

namespace mmotd::platform {

SystemDetails GetSystemInformationDetails() {
    auto kernel_details_holder = GetKernelDetails();
    if (!kernel_details_holder) {
        return SystemDetails{};
    }

    auto os_version_holder = GetOsVersion();
    if (!os_version_holder) {
        return SystemDetails{};
    }

    auto kernel_details = *kernel_details_holder;
    auto details = SystemDetails{};
    details.host_name = kernel_details.host_name;
    details.computer_name = kernel_details.computer_name;
    details.kernel_version = kernel_details.kernel_version.version;
    details.kernel_release = kernel_details.kernel_version.release.to_string();
    details.kernel_type = mmotd::system::to_string(kernel_details.kernel);
    details.architecture_type = mmotd::system::to_string(kernel_details.architecture);
    details.byte_order = mmotd::system::to_string(kernel_details.endian);

    auto [platform_name, major, minor, patch] = *os_version_holder;
    details.platform_version = format(FMT_STRING("{}.{:02}.{}"), major, minor, patch);
    details.platform_name = platform_name;

    return details;
}

} // namespace mmotd::platform
