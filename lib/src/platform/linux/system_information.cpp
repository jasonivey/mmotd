// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__linux__)
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include "lib/include/platform/system_information.h"
#include "lib/include/system_details.h"
#include "common/include/posix_error.h"
#include "common/include/iostream_error.h"

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

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
        PLOG_ERROR << format("error calling uname, {}", error_str);
        return nullopt;
    }

    auto sys_name = string(buf.sysname);
    auto node_name = string(buf.nodename);
    auto release = string(buf.release);
    auto version = string(buf.version);
    auto machine = string(buf.machine);
    auto domain_name = string(buf.domainname);

    PLOG_DEBUG << "sys name      (kernel) : " << sys_name;
    PLOG_DEBUG << "node name       (host) : " << node_name;
    PLOG_DEBUG << "release      (release) : " << release;
    PLOG_DEBUG << "version      (version) : " << version;
    PLOG_DEBUG << "machine (architecture) : " << machine;
    PLOG_DEBUG << "domain name            : " << domain_name;

    auto kernel_details = mmotd::system::KernelDetails::from_string(sys_name, node_name, release, version, machine);
    PLOG_DEBUG << "host name              : " << kernel_details.host_name;
    PLOG_DEBUG << "kernel version         : " << kernel_details.kernel_version.version;
    PLOG_DEBUG << "kernel release:        : " << kernel_details.kernel_version.release.to_string();
    PLOG_DEBUG << "kernel type            : " << mmotd::system::to_string(kernel_details.kernel);
    PLOG_DEBUG << "architecture           : " << mmotd::system::to_string(kernel_details.architecture);
    PLOG_DEBUG << "endian type            : " << mmotd::system::to_string(kernel_details.endian);

    return make_optional(kernel_details);
}

constexpr static const char *OS_RELEASE = "/etc/os-release";

vector<string> GetOsVersionFile() {
    auto os_release_path = fs::path(OS_RELEASE);
    if (!fs::is_regular_file(os_release_path) || !fs::is_symlink(os_release_path)) {
        PLOG_ERROR << format("{} release file does not exist", OS_RELEASE);
        return vector<string>{};
    }

    auto ifs = ifstream{};
    ifs.exceptions(std::ifstream::goodbit);
    ifs.open(os_release_path, ios_base::in);
    if (!ifs.is_open() || ifs.fail() || ifs.bad()) {
        auto ifs_err = mmotd::error::ios_flags::to_string(ifs);
        PLOG_ERROR << format("unable to open {}, {}", OS_RELEASE, ifs_err);
        return vector<string>{};
    }

    auto version_lines = vector<string>{};
    for (string version_line; getline(ifs, version_line);) {
        if (ifs.fail() || ifs.bad()) {
            auto ifs_err = mmotd::error::ios_flags::to_string(ifs);
            PLOG_ERROR << format("error reading {}, {}", OS_RELEASE, ifs_err);
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
        PLOG_DEBUG << format("found integer offset for {} to be {}", updated_version_str, offset);
        updated_version_str = updated_version_str.substr(offset);
    }
    if (updated_version_str.empty()) {
        PLOG_DEBUG << format("stripped front of version string {} and now it's empty", version_str);
        return nullopt;
    }

    auto j = find_if(begin(updated_version_str), end(updated_version_str), [&digit_classifier](auto version_char) {
        return !digit_classifier(version_char);
    });

    if (j != end(updated_version_str)) {
        auto offset = std::distance(begin(updated_version_str), j);
        PLOG_DEBUG << format("found integer offset for {} to be {}", updated_version_str, offset);
        updated_version_str = updated_version_str.substr(0, offset);
    }
    if (updated_version_str.empty()) {
        PLOG_DEBUG << format("stripped back of version string {} and now it's empty", version_str);
        return nullopt;
    }

    PLOG_DEBUG << format("stripped version string is {}", updated_version_str);

    return make_optional(std::stoi(updated_version_str));
}

optional<tuple<int, int, int>> ParseOsVersion(const string &version_str) {
    auto version_numbers = vector<string>{};
    boost::split(version_numbers, version_str, boost::is_any_of("."), boost::token_compress_on);
    if (version_numbers.size() < 2) {
        PLOG_ERROR << format("unable to split '{}' into a version string", version_str);
        return nullopt;
    }
    int major, minor, patch = 0;
    auto major_wrapper = ParseIndividualOsVersion(version_numbers[0]);
    if (major_wrapper) {
        major = *major_wrapper;
    }
    auto minor_wrapper = ParseIndividualOsVersion(version_numbers[1]);
    if (minor_wrapper) {
        minor = *minor_wrapper;
    }
    if (version_numbers.size() > 2) {
        auto patch_wrapper = ParseIndividualOsVersion(version_numbers[2]);
        if (patch_wrapper) {
            patch = *patch_wrapper;
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
    //File: /etc/os-release
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
    //
    // Welcome to Ubuntu 20.04.1 LTS focal (GNU/Linux 5.4.0-58-generic x86_64)
    // Welcome to macOS 10 Catalina 10.15.7.19H15 (Darwin 19.6.0 x86_64)
    auto name = string{};
    auto codename = string{};
    int major, minor, patch = 0;
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
    return make_optional(make_tuple(format("{} {}", name, codename), major, minor, patch));

}

}

namespace mmotd::platform {

SystemInformationDetails GetSystemInformationDetails() {
    auto kernel_details_wrapper = GetKernelDetails();
    if (!kernel_details_wrapper) {
        return SystemInformationDetails{};
    }
    auto kernel_details = *kernel_details_wrapper;

    auto os_version_wrapper = GetOsVersion();
    if (!os_version_wrapper) {
        return SystemInformationDetails{};
    }
    auto [platform_name, major, minor, patch] = *os_version_wrapper;

    constexpr static const char *SYS_INFO = "system information";
    auto details = SystemInformationDetails{};

    details.emplace_back(make_tuple(SYS_INFO, format("host name: {}", kernel_details.host_name)));
    details.emplace_back(make_tuple(SYS_INFO, format("kernel version: {}", kernel_details.kernel_version.version)));
    details.emplace_back(make_tuple(SYS_INFO, format("kernel release: {}", kernel_details.kernel_version.release.to_string())));
    details.emplace_back(make_tuple(SYS_INFO, format("kernel type: {}", mmotd::system::to_string(kernel_details.kernel))));
    details.emplace_back(make_tuple(SYS_INFO, format("architecture: {}", mmotd::system::to_string(kernel_details.architecture))));
    details.emplace_back(make_tuple(SYS_INFO, format("byte order: {}", mmotd::system::to_string(kernel_details.endian))));
    details.emplace_back(make_tuple(SYS_INFO, format("platform version: {}.{:02}.{}", major, minor, patch)));
    details.emplace_back(make_tuple(SYS_INFO, format("platform name: {}", platform_name)));
    return details;
}

}

#endif
