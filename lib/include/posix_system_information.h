// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "lib/include/information_provider.h"

#include <array>
#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace mmotd {

enum class ArchitectureType {
    x64,
    arm,
    itanium,
    x86,
    unknown,
};

std::string to_string(ArchitectureType architecture);

enum class EndianType {
    little,
    big,
    unknown,
};

std::string to_string(EndianType endian);

enum class KernelType { unknown, linux, darwin };

std::string to_string(KernelType kernel_type);

struct KernelRelease {
    std::optional<uint32_t> major;
    std::optional<uint32_t> minor;
    std::optional<uint32_t> patch;
    std::optional<uint32_t> build;

    std::string to_string() const;
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

class PosixSystemInformation : public InformationProvider {
public:
    PosixSystemInformation() = default;

    std::string GetName() const override { return std::string{"posix system information"}; }
    bool QueryInformation() override;
    std::optional<mmotd::ComputerValues> GetInformation() const override;

private:
    std::optional<KernelDetails> kernel_details_ = {};
};

} // namespace mmotd
