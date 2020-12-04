// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/posix_system_information.h"

#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

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
        kernel_details_ = GetKernelDetails();
    }
    return kernel_details_.has_value();
}

optional<mmotd::ComputerValues> PosixSystemInformation::GetInformation() const {
    if (!kernel_details_) {
        PLOG_ERROR << "unable to return any posix system information because there isn't any gathered";
        return nullopt;
    }
    auto computer_values = ComputerValues{};
    const auto &kernel_details = *kernel_details_;
    computer_values.emplace_back(make_tuple("host name", kernel_details.host_name));
    computer_values.emplace_back(make_tuple("kernel version", kernel_details.kernel_version.version));
    computer_values.emplace_back(make_tuple("kernel release", mmotd::to_string(kernel_details.kernel_version.release)));
    computer_values.emplace_back(make_tuple("kernel type", mmotd::to_string(kernel_details.kernel)));
    computer_values.emplace_back(make_tuple("architecture", mmotd::to_string(kernel_details.architecture)));
    computer_values.emplace_back(make_tuple("byte-order", mmotd::to_string(kernel_details.endian)));
    return make_optional(computer_values);
}

ostream &operator<<(ostream &out, const mmotd::PosixSystemInformation &system_information) {
    out << mmotd::to_string(system_information);
    return out;
}

string mmotd::to_string(const mmotd::KernelRelease &kernel_release) {
    auto str = string{};
    if (kernel_release.major) {
        str += std::to_string(kernel_release.major.value());
    } else {
        return str;
    }
    if (kernel_release.minor) {
        str += "." + std::to_string(kernel_release.minor.value());
    } else {
        return str;
    }
    if (kernel_release.patch) {
        str += "." + std::to_string(kernel_release.patch.value());
    } else {
        return str;
    }
    if (kernel_release.build) {
        str += "." + std::to_string(kernel_release.build.value());
    } else {
        return str;
    }
    return str;
}

ostream &operator<<(ostream &out, const mmotd::KernelRelease &kernel_release) {
    out << format("kernel release: {}\n", mmotd::to_string(kernel_release));
    return out;
}

string mmotd::to_string(const mmotd::KernelVersion &kernel_version) {
    auto str = mmotd::to_string(kernel_version.release);
    str += "\n" + kernel_version.version;
    return str;
}

ostream &operator<<(ostream &out, const mmotd::KernelVersion &kernel_version) {
    out << format("kernel version: {}\n", to_string(kernel_version.version));
    out << mmotd::to_string(kernel_version.release);
    return out;
}

string mmotd::to_string(const mmotd::KernelDetails &kernel_details) {
    auto str = to_string(kernel_details.kernel);
    str += "\n" + mmotd::to_string(kernel_details.kernel_version);
    str += "\n" + kernel_details.host_name;
    str += "\n" + to_string(kernel_details.architecture);
    str += "\n" + to_string(kernel_details.endian);
    return str;
}

ostream &operator<<(ostream &out, const mmotd::KernelDetails &kernel_details) {
    // out << format("kernel type: {}\n", to_string(kernel_details.kernel));
    out << to_string(kernel_details.kernel) << "\n";
    out << mmotd::to_string(kernel_details.kernel_version);
    out << format("host name: {}\n", to_string(kernel_details.host_name));
    // out << format("architecture: {}\n", to_string(kernel_details.architecture));
    out << to_string(kernel_details.architecture) << "\n";
    out << to_string(kernel_details.endian) << "\n";
    return out;
}

string mmotd::PosixSystemInformation::to_string() const {
    if (kernel_details_) {
        return mmotd::to_string(*kernel_details_);
    } else {
        return string{};
    }
}

string mmotd::to_string(const mmotd::PosixSystemInformation &system_information) {
    return system_information.to_string();
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

ostream &operator<<(ostream &out, const mmotd::KernelType &kernel_type) {
    out << format("kernel type: {}", mmotd::to_string(kernel_type));
    return out;
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
ostream &operator<<(ostream &out, const mmotd::ArchitectureType &architecture) {
    out << "architecture: " << mmotd::to_string(architecture);
    return out;
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

ostream &operator<<(ostream &out, const mmotd::EndianType &endian) {
    out << "Endian: " << mmotd::to_string(endian);
    return out;
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
