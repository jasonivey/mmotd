// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__APPLE__)
#include "common/assertion/include/precondition.h"
#include "common/include/logging.h"
#include "common/include/posix_error.h"
#include "lib/include/platform/system_information.h"
#include "lib/include/system_details.h"

#include <array>
#include <cstddef>
#include <limits>
#include <optional>
#include <tuple>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

#include <sys/sysctl.h>
#include <sys/utsname.h>

using namespace fmt;
using namespace std;
using namespace mmotd::system;

namespace {

template<typename T>
string GetPlatformIdentifier(int major, int minor, const T &version_map) {
    for (const auto &[major_version, minor_version, id] : version_map) {
        if (major == major_version && minor <= minor_version) {
            return string(id);
        }
    }
    return string{"unknown"};
}

string GetPlatformNameVersion(int major, int minor) {
    static constexpr std::array<std::tuple<int, int, const char *>, 3> PLATFORM_NAMES = {
        make_tuple(10, 11, "Mac OS X"),
        make_tuple(10, 15, "macOS 10"),
        make_tuple(11, numeric_limits<int>::max(), "macOS 11")};
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
        make_tuple(11, numeric_limits<int>::max(), "Big Sur")};

    auto platform_name = GetPlatformIdentifier(major, minor, PLATFORM_NAMES);
    auto version_name = GetPlatformIdentifier(major, minor, VERSION_NAMES);

    return format(FMT_STRING("{} {}"), platform_name, version_name);
}

optional<tuple<int, int, int>> GetOsVersion() {
    auto version_buf = array<char, 64>{};
    auto buf_size = std::size(version_buf);
    if (sysctlbyname("kern.osproductversion", std::data(version_buf), &buf_size, nullptr, 0) == -1) {
        auto error_str = mmotd::error::posix_error::to_string();
        LOG_ERROR("error calling sysctlbyname with kern.osproductversion, details: {}", error_str);
        return nullopt;
    }
    auto versions = vector<string>{};
    boost::split(versions, version_buf, boost::is_any_of("."));
    auto version_numbers = vector<int>(size_t{3}, int{0});
    if (!versions.empty()) {
        version_numbers[0] = stoi(versions[0]);
    }
    if (versions.size() > 1) {
        version_numbers[1] = stoi(versions[1]);
    }
    if (versions.size() > 2) {
        version_numbers[2] = stoi(versions[2]);
    }
    return {make_tuple(version_numbers[0], version_numbers[1], version_numbers[2])};
}

optional<KernelDetails> GetKernelDetails() {
    struct utsname buf = {};
    int retval = uname(&buf);
    LOG_DEBUG("uname returned {} [{}]", retval, retval == 0 ? "success" : "failed");
    if (retval != 0) {
        LOG_ERROR("uname failed with return code '{}'", retval);
        return nullopt;
    }

    auto sys_name = string(buf.sysname);
    auto node_name = string(buf.nodename);
    auto release = string(buf.release);
    auto version = string(buf.version);
    auto machine = string(buf.machine);

    LOG_DEBUG("sys name: {}", sys_name);
    LOG_DEBUG("node name: {}", node_name);
    LOG_DEBUG("release: {}", release);
    LOG_DEBUG("version: {}", version);
    LOG_DEBUG("machine: {}", machine);

    return KernelDetails::from_string(sys_name, node_name, release, version, machine);
}

} // namespace

namespace mmotd::platform {

SystemDetails GetSystemInformationDetails() {
    auto kernel_details_holder = GetKernelDetails();
    if (!kernel_details_holder) {
        return SystemDetails{};
    }

    auto details = SystemDetails{};
    auto kernel_details = *kernel_details_holder;
    details.host_name = kernel_details.host_name;
    details.computer_name = kernel_details.computer_name;
    details.kernel_version = kernel_details.kernel_version.version;
    details.kernel_release = kernel_details.kernel_version.release.to_string();
    details.kernel_type = mmotd::system::to_string(kernel_details.kernel);
    details.architecture_type = mmotd::system::to_string(kernel_details.architecture);

    auto os_version_holder = GetOsVersion();
    if (!os_version_holder) {
        return SystemDetails{};
    }

    auto [major, minor, patch] = *os_version_holder;
    details.platform_version = format(FMT_STRING("{}.{}.{}"), major, minor, patch);
    details.platform_name = GetPlatformNameVersion(major, minor);

    return details;
}

} // namespace mmotd::platform
#endif
