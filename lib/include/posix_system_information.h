// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <array>
#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

enum class ArchitectureType {
    x64,
    arm,
    itanium,
    x86,
    unknown,
};

enum class EndianType {
    little,
    big,
    unknown,
};

enum class KernelType {
    unknown,
    linux,
    darwin
};

struct KernelRelease {
    std::optional<uint32_t> major;
    std::optional<uint32_t> minor;
    std::optional<uint32_t> patch;
    std::optional<uint32_t> build;

};

struct KernelVersion {
    std::string version;
    KernelRelease release;
};

struct KernelDetails {
    std::string host_name;
    KernelVersion kernel_version;
    KernelType kernel = KernelType::unknown;
    ArchitectureType architecture = ArchitectureType::unknown;
    EndianType endian = EndianType::unknown;
};

class PosixSystemInformation {
    friend std::ostream &operator<<(std::ostream &out, const PosixSystemInformation &system_information);
public:
    PosixSystemInformation() = default;

    std::string to_string() const;
    bool TryDiscovery();

private:
    KernelDetails kernel_details_ = {};
};

std::string to_string(ArchitectureType architecture);
std::string to_string(EndianType endian);
std::string to_string(KernelType kernel_type);
std::string to_string(const KernelRelease &kernel_release);
std::string to_string(const KernelVersion &kernel_version);
std::string to_string(const KernelDetails &kernel_details);
std::string to_string(const PosixSystemInformation &system_information);


std::ostream &operator<<(std::ostream &out, const ArchitectureType &architecture);
std::ostream &operator<<(std::ostream &out, const EndianType &endian);
std::ostream &operator<<(std::ostream &out, const KernelType &kernel_type);
std::ostream &operator<<(std::ostream &out, const KernelRelease &kernel_release);
std::ostream &operator<<(std::ostream &out, const KernelVersion &kernel_version);
std::ostream &operator<<(std::ostream &out, const KernelDetails &kernel_details);
std::ostream &operator<<(std::ostream &out, const PosixSystemInformation &system_information);
