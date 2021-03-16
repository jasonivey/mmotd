// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/posix_error.h"
#include "lib/include/system_details.h"

#include <optional>
#include <sstream>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

using namespace fmt;
using namespace std;

namespace {

string TransformHostNameToComputerName(string host_name) {
    if (empty(host_name)) {
        return string{};
    }
    auto index = host_name.find('.');
    return host_name.substr(0, index);
}

} // namespace

namespace mmotd::system {

string KernelRelease::to_string() const {
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
        str += format("{}{}", (build.value()[0] == '-' ? "" : "."), build.value());
    } else {
        return str;
    }
    return str;
}

string to_string(KernelType kernel_type) {
    switch (kernel_type) {
        case KernelType::linux:
            return "GNU/Linux";
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

string to_string(ArchitectureType architecture) {
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

string to_string(EndianType endian) {
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

ArchitectureType to_architecture_type(const string &type_str) {
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

EndianType detect_endian_type() {
    const uint16_t test = 0xFF00;
    const auto result = *static_cast<const uint8_t *>(static_cast<const void *>(&test));

    if (result == 0xFF) {
        return EndianType::big;
    } else {
        return EndianType::little;
    }
}

KernelType to_kernel_type(const string &type_str) {
    if (type_str == "Linux" || type_str == "GNU/Linux") {
        return KernelType::linux;
    } else if (type_str == "Darwin") {
        return KernelType::darwin;
    } else {
        return KernelType::unknown;
    }
}

KernelRelease KernelRelease::from_string(const string &release) {
    auto release_parts = vector<string>{};
    istringstream ss(release);
    for (string part; getline(ss, part, '.');) {
        release_parts.push_back(part);
    }
    if (release_parts.size() < 2) {
        auto error_str = format("uname release string is not of the format 'xx.yy' instead it is '{}'", release);
        MMOTD_THROW_RUNTIME_ERROR(error_str);
    }
    auto kernel_release = KernelRelease{};
    kernel_release.major = stoul(release_parts[0]);
    kernel_release.minor = stoul(release_parts[1]);
    if (release_parts.size() > 2) {
        if (auto i = release_parts[2].find('-'); i != string::npos) {
            kernel_release.build = release_parts[2].substr(i);
            release_parts[2] = release_parts[2].substr(0, i);
        }
        if (std::all_of(begin(release_parts[2]), end(release_parts[2]), boost::is_digit())) {
            kernel_release.patch = stoul(release_parts[2]);
        }
    }
    if (release_parts.size() > 3) {
        if (std::all_of(begin(release_parts[3]), end(release_parts[3]), boost::is_digit())) {
            kernel_release.build = release_parts[3];
        }
    }
    return kernel_release;
}

KernelVersion KernelVersion::from_string(const string &full_version, const string &full_release) {
    auto kernel_version = KernelVersion{};
    auto index = full_version.find_first_of(':');
    if (index != string::npos) {
        kernel_version.version = full_version.substr(0, index);
    } else {
        kernel_version.version = full_version;
    }
    kernel_version.release = KernelRelease::from_string(full_release);
    return kernel_version;
}

KernelDetails KernelDetails::from_string(const string &kernel_type,
                                         const string &host_name,
                                         const string &release,
                                         const string &version,
                                         const string &architecture) {
    auto kernel_details = KernelDetails{};
    kernel_details.kernel = to_kernel_type(kernel_type);
    kernel_details.kernel_version = KernelVersion::from_string(version, release);
    kernel_details.host_name = host_name;
    kernel_details.computer_name = TransformHostNameToComputerName(host_name);
    kernel_details.architecture = to_architecture_type(architecture);
    kernel_details.endian = detect_endian_type();
    return kernel_details;
}

} // namespace mmotd::system
