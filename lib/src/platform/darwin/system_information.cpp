// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__APPLE__)

#include "common/include/posix_error.h"
#include "lib/include/platform/system_information.h"

#include <optional>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

#include <sys/sysctl.h>
#include <sys/utsname.h>

using namespace fmt;
using namespace std;
using namespace mmotd::system;

namespace {

string GetPlatformName(int major, int minor) {
    static constexpr std::array<std::tuple<int, int, const char *>, 3> PLATFORM_NAMES = {make_tuple(10, 11, "Mac OS X"),
                                                                                         make_tuple(10, 15, "macOS 10"),
                                                                                         make_tuple(11, 0, "macOS 11")};
    static constexpr std::array<std::tuple<int, int, const char *>, 17> VERSION_NAMES = {
        make_tuple(10, 0, "Cheetah"),
        make_tuple(10, 1, "Puma"),
        make_tuple(10, 2, "Jaguar"),
        make_tuple(10, 3, "Panther"),
        make_tuple(10, 4, "Tiger"),
        make_tuple(10, 5, "Leopard"),
        make_tuple(10, 6, "Snow Leopard"),
        make_tuple(10, 7, "Lion"),
        make_tuple(10, 8, "Mountain Lion"),
        make_tuple(10, 9, "Mavericks"),
        make_tuple(10, 10, "Yosemite"),
        make_tuple(10, 11, "El Capitan"),
        make_tuple(10, 12, "Sierra"),
        make_tuple(10, 13, "High Sierra"),
        make_tuple(10, 14, "Mojave"),
        make_tuple(10, 15, "Catalina"),
        make_tuple(11, 0, "Big Sur")};

    auto platform_name = string{"unknown"};
    for (const auto &platform_name_pack : PLATFORM_NAMES) {
        auto [major_version, minor_version, platform] = platform_name_pack;
        if (major == major_version && minor <= minor_version) {
            platform_name = string{platform};
            break;
        }
    }

    auto version_name = string{"unkown"};
    for (const auto &version_name_pack : VERSION_NAMES) {
        auto [major_version, minor_version, version] = version_name_pack;
        if (major == major_version && minor == minor_version) {
            version_name = string{version};
            break;
        }
    }

    return format("{} {}", platform_name, version_name);
}

optional<tuple<int, int, int>> GetOsVersion() {
    int mib[4] = {0, 0, 0, 0};
    size_t miblen = 512;
    if (sysctlnametomib("kern.osproductversion", mib, &miblen) == -1) {
        auto error_str = mmotd::error::posix_error::to_string();
        PLOG_ERROR << format("error calling sysctlnametomib with kern.osproductversion, details: {}", error_str);
        return nullopt;
    }
    char version[512] = {0};
    memset(version, 0, 512);
    size_t length = sizeof(long long);
    if (sysctl(mib, 2, version, &length, NULL, 0) == -1) {
        auto error_str = mmotd::error::posix_error::to_string();
        PLOG_ERROR << format("error calling sysctl with kern.osproductversion, details: {}", error_str);
        return nullopt;
    }
    auto versions = vector<string>{};
    boost::split(versions, version, boost::is_any_of("."));
    auto version_numbers = vector<int>{0, 0, 0};
    if (!versions.empty()) {
        version_numbers[0] = stoi(versions[0]);
    }
    if (versions.size() > 1) {
        version_numbers[1] = stoi(versions[1]);
    }
    if (versions.size() > 2) {
        version_numbers[2] = stoi(versions[2]);
    }
    return make_optional(make_tuple(version_numbers[0], version_numbers[1], version_numbers[2]));
}

optional<KernelDetails> GetKernelDetails() {
    struct utsname buf = {};
    int retval = uname(&buf);
    PLOG_DEBUG << format("uname returned {} [{}]", retval, retval == 0 ? "success" : "failed");
    if (retval != 0) {
        PLOG_ERROR << format("uname failed with return code '{}'", retval);
        return nullopt;
    }

    auto sys_name = string(buf.sysname);
    auto node_name = string(buf.nodename);
    auto release = string(buf.release);
    auto version = string(buf.version);
    auto machine = string(buf.machine);

    PLOG_DEBUG << "sys name: " << sys_name;
    PLOG_DEBUG << "node name: " << node_name;
    PLOG_DEBUG << "release: " << release;
    PLOG_DEBUG << "version: " << version;
    PLOG_DEBUG << "machine: " << machine;

    return to_kernel_details(sys_name, node_name, release, version, machine);
}

}

namespace mmotd::platform {

SystemInformationDetails GetSystemInformationDetails() {
    auto details = SystemInformationDetails{};
    auto kernel_details_wrapper = GetKernelDetails();
    if (kernel_details_wrapper) {
        auto kernel_details = *kernel_details_wrapper;
        details.emplace_back(make_tuple("system information", format("host name: {}", kernel_details.host_name)));
        details.emplace_back(
            make_tuple("system information", format("kernel version: {}", kernel_details.kernel_version.version)));
        details.emplace_back(
            make_tuple("system information",
                       format("kernel release: {}", kernel_details.kernel_version.release.to_string())));
        details.emplace_back(
            make_tuple("system information", format("kernel type: {}", mmotd::system::to_string(kernel_details.kernel))));
        details.emplace_back(make_tuple("system information",
                                         format("architecture: {}", mmotd::system::to_string(kernel_details.architecture))));
        details.emplace_back(
            make_tuple("system information", format("byte order: {}", mmotd::system::to_string(kernel_details.endian))));
    }
    auto os_version = GetOsVersion();
    if (os_version) {
        auto [major, minor, patch] = *os_version;
        details.emplace_back(
            make_tuple("system information", format("platform version: {}.{}.{}", major, minor, patch)));
        details.emplace_back(
            make_tuple("system information", format("platform name: {}", GetPlatformName(major, minor))));
    }
    //for (const auto &computer_value : details) {
    //    const auto &[name, value] = computer_value;
    //    PLOG_VERBOSE << format("{}: {}", name, value);
    //}
    return details;
}

}

#endif
