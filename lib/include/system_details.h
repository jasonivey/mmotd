// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace mmotd::system {

enum class ArchitectureType {
    x64,
    arm,
    itanium,
    x86,
    unknown,
};

ArchitectureType to_architecture_type(const std::string &type_str);
std::string to_string(ArchitectureType architecture);

enum class EndianType {
    little,
    big,
    unknown,
};

EndianType detect_endian_type();
std::string to_string(EndianType endian);

enum class KernelType { unknown, linux, darwin };

KernelType to_kernel_type(const std::string &type_str);
std::string to_string(KernelType kernel_type);

struct KernelRelease {
    std::optional<uint32_t> major;
    std::optional<uint32_t> minor;
    std::optional<uint32_t> patch;
    std::optional<std::string> build;

    std::string to_string() const;
    static KernelRelease from_string(const std::string &release);
};

struct KernelVersion {
    std::string version;
    KernelRelease release;

    static KernelVersion from_string(const std::string &full_version, const std::string &full_release);
};

struct KernelDetails {
    std::string host_name;
    KernelVersion kernel_version;
    KernelType kernel = KernelType::unknown;
    ArchitectureType architecture = ArchitectureType::unknown;
    EndianType endian = EndianType::unknown;

    static KernelDetails from_string(const std::string &kernel_type,
                                     const std::string &host_name,
                                     const std::string &release,
                                     const std::string &version,
                                     const std::string &architecture);
};

} // namespace mmotd::system
