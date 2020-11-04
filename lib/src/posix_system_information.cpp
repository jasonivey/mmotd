// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/posix_system_information.h"

#include <stdexcept>
#include <sys/utsname.h>

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <fmt/format.h>
#include <iostream>
#include <optional>
#include <iostream>
#include <sstream>

using namespace fmt;
using namespace std;

static KernelRelease to_kernel_release(const std::string &release);
static KernelVersion to_kernel_version(const std::string &full_version, const std::string &full_release);
static KernelType to_kernel_type(const std::string &type_str);
static EndianType to_endian_type();
static ArchitectureType to_architecture_type(const std::string &type_str);
static KernelDetails to_kernel_details(const string &kernel_type,
                                       const string &host_name,
                                       const string &release,
                                       const string &version,
                                       const string &architecture);

static optional<KernelDetails> GetKernelDetails() {
    struct utsname buf = {};
    int retval = uname(&buf);
    BOOST_LOG_TRIVIAL(debug) << format("uname returned {} [{}]", retval, retval == 0 ? "success" : "failed");
    if (retval != 0) {
        BOOST_LOG_TRIVIAL(error) << format("uname failed with return code '{}'", retval);
        return optional<KernelDetails>{};
    }

    auto sys_name = string(buf.sysname);
    auto node_name = string(buf.nodename);
    auto release = string(buf.release);
    auto version = string(buf.version);
    auto machine = string(buf.machine);

    BOOST_LOG_TRIVIAL(debug) << "sys name: " << sys_name;
    BOOST_LOG_TRIVIAL(debug) << "node name: " << node_name;
    BOOST_LOG_TRIVIAL(debug) << "release: " << release;
    BOOST_LOG_TRIVIAL(debug) << "version: " << version;
    BOOST_LOG_TRIVIAL(debug) << "machine: " << machine;

    return to_kernel_details(sys_name, node_name, release, version, machine);
}

bool PosixSystemInformation::TryDiscovery() {
    const auto kernel_details = GetKernelDetails();
    if (kernel_details) {
        kernel_details_ = *kernel_details;
        return true;
    }
    return false;
}


ostream &operator<<(ostream &out, const PosixSystemInformation &system_information) {
    out << system_information.kernel_details_;
    return out;
}

string to_string(const KernelRelease &kernel_release) {
    auto str = string{};
    if (kernel_release.major) {
        str += to_string(*kernel_release.major);
    } else {
        return str;
    }
    if (kernel_release.minor) {
        str += "." + to_string(*kernel_release.minor);
    } else {
        return str;
    }
    if (kernel_release.patch) {
        str += "." + to_string(*kernel_release.patch);
    } else {
        return str;
    }
    if (kernel_release.build) {
        str += "." + to_string(*kernel_release.build);
    } else {
        return str;
    }
    return str;
}

ostream &operator<<(ostream &out, const KernelRelease &kernel_release) {
    out << format("kernel release: {}\n", to_string(kernel_release));
    return out;
}

string to_string(const KernelVersion &kernel_version) {
    auto str = to_string(kernel_version.release);
    str += "\n" + to_string(kernel_version.version);
    return str;
}

ostream &operator<<(ostream &out, const KernelVersion &kernel_version) {
    out << format("kernel version: {}\n", to_string(kernel_version.version));
    out << kernel_version.release;
    return out;
}

string to_string(const KernelDetails &kernel_details) {
    auto str = to_string(kernel_details.kernel);
    str += "\n" + to_string(kernel_details.kernel_version);
    str += "\n" + to_string(kernel_details.host_name);
    str += "\n" + to_string(kernel_details.architecture);
    str += "\n" + to_string(kernel_details.endian);
    return str;
}

ostream &operator<<(ostream &out, const KernelDetails &kernel_details) {
    //out << format("kernel type: {}\n", to_string(kernel_details.kernel));
    out << kernel_details.kernel << "\n";
    out << kernel_details.kernel_version;
    out << format("host name: {}\n", to_string(kernel_details.host_name));
    //out << format("architecture: {}\n", to_string(kernel_details.architecture));
    out << kernel_details.architecture << "\n";
    out << kernel_details.endian << "\n";
    return out;
}

string PosixSystemInformation::to_string() const {
    return ::to_string(kernel_details_);
}

string to_string(const PosixSystemInformation &system_information) {
    return system_information.to_string();
}

string to_string(KernelType kernel_type) {
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

ostream &operator<<(ostream &out, const KernelType &kernel_type) {
    out << format("kernel type: {}", to_string(kernel_type));
    return out;
}

std::string to_string(ArchitectureType architecture) {
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
ostream &operator<<(ostream &out, const ArchitectureType &architecture) {
    out << "architecture: " << to_string(architecture);
    return out;
}

std::string to_string(EndianType endian) {
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

ostream &operator<<(ostream &out, const EndianType &endian) {
    out << "Endian: " << to_string(endian);
    return out;
}

static ArchitectureType to_architecture_type(const std::string &type_str) {
    if (boost::iequals(type_str, "x86_64")) {
        return ArchitectureType::x64;
    } else if (boost::istarts_with(type_str, "arm"))
        return ArchitectureType::arm;
    else if (boost::iequals(type_str, "ia64"))
        return ArchitectureType::itanium;
    else if(boost::iequals(type_str, "i686"))
        return ArchitectureType::x86;
    else
        return ArchitectureType::unknown;
}

static EndianType to_endian_type() {
    const std::uint16_t test = 0xFF00;
    const auto result = *static_cast<const std::uint8_t*>(static_cast<const void*>(&test));

    if (result == 0xFF)
        return EndianType::big;
    else
        return EndianType::little;
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
    std::istringstream ss(release);
    for (string part; getline(ss, part, '.');) {
        release_parts.push_back(part);
    }
    if (release_parts.size() < 2) {
        auto error_str = format("uname release string is not of the format 'xx.yy' instead it is '{}'", release);
        BOOST_LOG_TRIVIAL(error) << error_str;
        throw std::runtime_error(error_str);
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

