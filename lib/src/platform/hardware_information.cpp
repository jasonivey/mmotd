// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/platform/hardware_information.h"

#include <string>

#include <boost/algorithm/string.hpp>

using namespace std;

namespace mmotd::platform {

string to_string(EndianType endian) {
    using namespace std::literals;
    switch (endian) {
        case EndianType::little:
            return "little endian"s;
        case EndianType::big:
            return "big endian"s;
        case EndianType::unknown:
        default:
            return "unknown"s;
    }
}

EndianType from_endian_string(string endian_str) {
    using namespace boost::algorithm;
    if (icontains(endian_str, "little")) {
        return EndianType::little;
    } else if (icontains(endian_str, "big")) {
        return EndianType::big;
    } else {
        return EndianType::unknown;
    }
}

bool HardwareDetails::empty() const noexcept {
    return std::empty(machine_type) && std::empty(machine_model) && std::empty(cpu_name) &&
           byte_order == EndianType::unknown && std::empty(gpu_name) && std::empty(monitor_name) &&
           std::empty(monitor_resolution);
}

} // namespace mmotd::platform
