// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/posix_error.h"
#include "lib/include/computer_information.h"
#include "lib/include/posix_system_information.h"

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
using namespace mmotd;

bool gLinkPosixSystemInformation = false;

static KernelRelease to_kernel_release(const string &release);
static KernelVersion to_kernel_version(const string &full_version, const string &full_release);
static KernelType to_kernel_type(const string &type_str);
static EndianType to_endian_type();
static ArchitectureType to_architecture_type(const string &type_str);
static KernelDetails to_kernel_details(const string &kernel_type,
                                       const string &host_name,
                                       const string &release,
                                       const string &version,
                                       const string &architecture);
static string GetPlatformName(int major, int minor);
static optional<tuple<int, int, int>> GetOsVersion();

static const bool posix_system_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::PosixSystemInformation>(); });

static optional<KernelDetails> GetKernelDetails() {
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

bool PosixSystemInformation::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        return GetSystemInformation();
    }
    return has_queried;
}

bool PosixSystemInformation::GetSystemInformation() {
    auto kernel_details_wrapper = GetKernelDetails();
    if (kernel_details_wrapper) {
        auto kernel_details = *kernel_details_wrapper;
        details_.emplace_back(make_tuple("system information", format("host name: {}", kernel_details.host_name)));
        details_.emplace_back(
            make_tuple("system information", format("kernel version: {}", kernel_details.kernel_version.version)));
        details_.emplace_back(
            make_tuple("system information",
                       format("kernel release: {}", kernel_details.kernel_version.release.to_string())));
        details_.emplace_back(
            make_tuple("system information", format("kernel type: {}", mmotd::to_string(kernel_details.kernel))));
        details_.emplace_back(make_tuple("system information",
                                         format("architecture: {}", mmotd::to_string(kernel_details.architecture))));
        details_.emplace_back(
            make_tuple("system information", format("byte order: {}", mmotd::to_string(kernel_details.endian))));
    }
    auto os_version = GetOsVersion();
    if (os_version) {
        auto [major, minor, patch] = *os_version;
        details_.emplace_back(
            make_tuple("system information", format("platform version: {}.{}.{}", major, minor, patch)));
        details_.emplace_back(
            make_tuple("system information", format("platform name: {}", GetPlatformName(major, minor))));
    }
    for (const auto &computer_value : details_) {
        const auto &[name, value] = computer_value;
        PLOG_VERBOSE << format("{}: {}", name, value);
    }
    return true;
}

optional<mmotd::ComputerValues> PosixSystemInformation::GetInformation() const {
    return !details_.empty() ? make_optional(details_) : nullopt;
}

string mmotd::KernelRelease::to_string() const {
    auto str = string{};
    if (major) {
        str += std::to_string(major.value());
    } else {
        return str;
    }
    if (minor) {
        str += "." + std::to_string(minor.value());
    } else {
        return str;
    }
    if (patch) {
        str += "." + std::to_string(patch.value());
    } else {
        return str;
    }
    if (build) {
        str += "." + std::to_string(build.value());
    } else {
        return str;
    }
    return str;
}

std::string mmotd::to_string(mmotd::KernelType kernel_type) {
    switch (kernel_type) {
        case KernelType::linux:
            return "Linux";
            break;
        case KernelType::darwin:
            return "Darwin";
            break;
        case KernelType::unknown:
        default:
            return "Unknown";
            break;
    }
    return string{};
}

string mmotd::to_string(mmotd::ArchitectureType architecture) {
    switch (architecture) {
        case ArchitectureType::x64:
            return "x86_64";
        case ArchitectureType::arm:
            return "arm";
        case ArchitectureType::itanium:
            return "itanium";
        case ArchitectureType::x86:
            return "x86";
        case ArchitectureType::unknown:
        default:
            return "unknown";
    }
}

string mmotd::to_string(mmotd::EndianType endian) {
    switch (endian) {
        case EndianType::little:
            return "little";
        case EndianType::big:
            return "big";
        case EndianType::unknown:
        default:
            return "unknown";
    }
}

static ArchitectureType to_architecture_type(const string &type_str) {
    if (boost::iequals(type_str, "x86_64")) {
        return ArchitectureType::x64;
    } else if (boost::istarts_with(type_str, "arm")) {
        return ArchitectureType::arm;
    } else if (boost::iequals(type_str, "ia64")) {
        return ArchitectureType::itanium;
    } else if (boost::iequals(type_str, "i686")) {
        return ArchitectureType::x86;
    } else {
        return ArchitectureType::unknown;
    }
}

static EndianType to_endian_type() {
    const uint16_t test = 0xFF00;
    const auto result = *static_cast<const uint8_t *>(static_cast<const void *>(&test));

    if (result == 0xFF) {
        return EndianType::big;
    } else {
        return EndianType::little;
    }
}

static KernelType to_kernel_type(const string &type_str) {
    if (type_str == "Linux") {
        return KernelType::linux;
    } else if (type_str == "Darwin") {
        return KernelType::darwin;
    } else {
        return KernelType::unknown;
    }
}

static KernelRelease to_kernel_release(const string &release) {
    auto release_parts = vector<string>{};
    istringstream ss(release);
    for (string part; getline(ss, part, '.');) {
        release_parts.push_back(part);
    }
    if (release_parts.size() < 2) {
        auto error_str = format("uname release string is not of the format 'xx.yy' instead it is '{}'", release);
        PLOG_ERROR << error_str;
        throw runtime_error(error_str);
    }
    auto kernel_release = KernelRelease{};
    kernel_release.major = stoul(release_parts[0]);
    kernel_release.minor = stoul(release_parts[1]);
    if (release_parts.size() > 2) {
        kernel_release.patch = stoul(release_parts[2]);
    }
    if (release_parts.size() > 3) {
        kernel_release.build = stoul(release_parts[3]);
    }
    return kernel_release;
}

static KernelVersion to_kernel_version(const string &full_version, const string &full_release) {
    auto kernel_version = KernelVersion{};
    auto index = full_version.find_first_of(':');
    if (index != string::npos) {
        kernel_version.version = full_version.substr(0, index);
    } else {
        kernel_version.version = full_version;
    }
    kernel_version.release = to_kernel_release(full_release);
    return kernel_version;
}

static KernelDetails to_kernel_details(const string &kernel_type,
                                       const string &host_name,
                                       const string &release,
                                       const string &version,
                                       const string &architecture) {
    auto kernel_details = KernelDetails{};
    kernel_details.kernel = to_kernel_type(kernel_type);
    kernel_details.kernel_version = to_kernel_version(version, release);
    kernel_details.host_name = host_name;
    kernel_details.architecture = to_architecture_type(architecture);
    kernel_details.endian = to_endian_type();
    return kernel_details;
}

static string GetPlatformName(int major, int minor) {
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

static optional<tuple<int, int, int>> GetOsVersion() {
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
