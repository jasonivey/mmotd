// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <cstdint>
#include <iterator>
#include <string>
#include "lib/include/platform/hardware_temperature.h"

namespace mmotd::platform {

enum class EndianType {
    little,
    big,
    unknown,
};

std::string to_string(EndianType endian);
EndianType from_endian_string(std::string endian_str);

struct HardwareDetails {
    std::string machine_type;
    std::string machine_model;
    std::int32_t cpu_core_count = 0;
    std::string cpu_name;
    EndianType byte_order = EndianType::unknown;
    Temperature cpu_temperature;
    std::string gpu_name;
    Temperature gpu_temperature;
    std::string monitor_name;
    std::string monitor_resolution;

    bool empty() const noexcept;
};

HardwareDetails GetHardwareInformationDetails();

} // namespace mmotd::platform
