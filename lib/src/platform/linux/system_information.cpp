// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__linux__)
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

    LOG_DEBUG("sys name      (kernel) : {}", sys_name);  // "Linux" (both)
    LOG_DEBUG("node name       (host) : {}", node_name); // "fedora" (hostname)
    LOG_DEBUG("release      (release) : {}", release);   // "5.16.11-200.fc35.x86_64" | "5.4.0-92-generic"
    // "#1 SMP PREEMPT Wed Feb 23 17:08:49 UTC 2022" | "#103-Ubuntu SMP Fri Nov 26 16:13:00 UTC 2021"
    LOG_DEBUG("version      (version) : {}", version);
    LOG_DEBUG("machine (architecture) : {}", machine);     // "x86_64"
    LOG_DEBUG("domain name            : {}", domain_name); // "(none)"

    auto kernel_details = mmotd::system::KernelDetails::from_string(sys_name, node_name, release, version, machine);
    LOG_DEBUG("host name              : {}", kernel_details.host_name);
    LOG_DEBUG("kernel version         : {}", kernel_details.kernel_version.version);
    LOG_DEBUG("kernel release:        : {}", kernel_details.kernel_version.release.to_string());
    LOG_DEBUG("kernel type            : {}", mmotd::system::to_string(kernel_details.kernel));
    LOG_DEBUG("architecture           : {}", mmotd::system::to_string(kernel_details.architecture));

    return make_optional(kernel_details);
}

static constexpr string_view OS_RELEASE = "/etc/os-release";

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

optional<int> ParseIndividualOsVersion(string version_str) {
    using boost::is_digit;

    auto i = find_if(begin(version_str), end(version_str), [](auto version_char) { return is_digit()(version_char); });

    if (i == end(version_str)) {
        LOG_WARNING("no integer found in version string '{}'", version_str);
        return nullopt;
    }

    auto offset = std::distance(begin(version_str), i);
    LOG_DEBUG("found integer offset for '{}' to be {}", version_str, offset);
    version_str = version_str.substr(offset);

    if (empty(version_str)) {
        LOG_DEBUG("stripped front of version string '{}' and now it's empty", version_str);
        return nullopt;
    }

    auto j = find_if(begin(version_str), end(version_str), [](auto version_char) { return !is_digit()(version_char); });

    if (j != end(version_str)) {
        offset = std::distance(begin(version_str), j);
        LOG_DEBUG("found integer offset for '{}' to be {}", version_str, offset);
        version_str = version_str.substr(0, offset);
    }
    if (empty(version_str)) {
        LOG_DEBUG("stripped back of version string '{}' and now it's empty", version_str);
        return nullopt;
    }

    LOG_DEBUG("stripped version string is '{}'", version_str);

    return make_optional(std::stoi(version_str));
}

struct VersionNumbers {
    optional<int> major;
    optional<int> minor;
    optional<int> patch;

    bool empty() const noexcept { return !major.has_value() && !minor.has_value() && !patch.has_value(); }
    explicit operator bool() const noexcept { return !empty(); }
    string to_string() const {
        if (major.has_value() && minor.has_value() && patch.has_value()) {
            return format(FMT_STRING("{}.{:02}.{}"), *major, *minor, *patch);
        } else if (major.has_value() && minor.has_value()) {
            return format(FMT_STRING("{}.{:02}"), *major, *minor);
        } else if (major.has_value()) {
            return ::to_string(*major);
        } else {
            return string{};
        }
    }
};

VersionNumbers ParseOsVersion(const string &version_str) {
    auto version_parts = vector<string>{};
    boost::split(version_parts, version_str, boost::is_any_of("."));
    auto version_numbers = VersionNumbers{};
    auto it = begin(version_parts);
    if (it != end(version_parts)) {
        version_numbers.major = ParseIndividualOsVersion(*it++);
    }
    if (it != end(version_parts)) {
        version_numbers.minor = ParseIndividualOsVersion(*it++);
    }
    if (it != end(version_parts)) {
        version_numbers.patch = ParseIndividualOsVersion(*it++);
    }
    return version_numbers;
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

// File (ubuntu): /etc/os-release
// NAME="Ubuntu"
// VERSION="20.04.3 LTS (Focal Fossa)"
// ID=ubuntu
// ID_LIKE=debian
// PRETTY_NAME="Ubuntu 20.04.3 LTS"
// VERSION_ID="20.04"
// HOME_URL="https://www.ubuntu.com/"
// SUPPORT_URL="https://help.ubuntu.com/"
// BUG_REPORT_URL="https://bugs.launchpad.net/ubuntu/"
// PRIVACY_POLICY_URL="https://www.ubuntu.com/legal/terms-and-policies/privacy-policy"
// VERSION_CODENAME=focal
// UBUNTU_CODENAME=focal

// File (fedora): /etc/os-release
// NAME="Fedora Linux"
// VERSION="35 (Server Edition)"
// ID=fedora
// VERSION_ID=35
// VERSION_CODENAME=""
// PLATFORM_ID="platform:f35"
// PRETTY_NAME="Fedora Linux 35 (Server Edition)"
// ANSI_COLOR="0;38;2;60;110;180"
// LOGO=fedora-logo-icon
// CPE_NAME="cpe:/o:fedoraproject:fedora:35"
// HOME_URL="https://fedoraproject.org/"
// DOCUMENTATION_URL="https://docs.fedoraproject.org/en-US/fedora/f35/system-administrators-guide/"
// SUPPORT_URL="https://ask.fedoraproject.org/"
// BUG_REPORT_URL="https://bugzilla.redhat.com/"
// REDHAT_BUGZILLA_PRODUCT="Fedora"
// REDHAT_BUGZILLA_PRODUCT_VERSION=35
// REDHAT_SUPPORT_PRODUCT="Fedora"
// REDHAT_SUPPORT_PRODUCT_VERSION=35
// PRIVACY_POLICY_URL="https://fedoraproject.org/wiki/Legal:PrivacyPolicy"
// VARIANT="Server Edition"
// VARIANT_ID=server

optional<tuple<string, VersionNumbers>> GetOsVersion() {
    // Output:
    // Welcome to Ubuntu 20.04.1 LTS focal (GNU/Linux 5.4.0-58-generic x86_64)
    // Welcome to macOS 10 Catalina 10.15.7.19H15 (Darwin 19.6.0 x86_64)
    auto name = string{};
    auto codename = string{};
    auto version_numbers = VersionNumbers{};

    const auto &file_data = GetOsVersionFile();
    for (const auto &file_line : file_data) {
        if (auto i = boost::ifind_first(file_line, "pretty_name="); !i.empty() && i.begin() == file_line.begin()) {
            version_numbers = ParseOsVersion(string{i.end(), file_line.end()});
        }
        if (auto i = boost::ifind_first(file_line, "name="); !i.empty() && i.begin() == file_line.begin()) {
            name = boost::trim_copy_if(string{i.end(), file_line.end()}, boost::is_any_of("\""));
        }
        if (auto i = boost::ifind_first(file_line, "version="); !i.empty() && i.begin() == file_line.begin()) {
            codename = ParseCodename(string{i.end(), file_line.end()});
        }
    }
    return make_optional(make_tuple(format(FMT_STRING("{} {}"), name, codename), version_numbers));
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

    auto [platform_name, version_numbers] = *os_version_holder;
    details.platform_version = version_numbers.to_string();
    details.platform_name = platform_name;

    return details;
}

} // namespace mmotd::platform
#endif
