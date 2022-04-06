// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/platform/hardware_information.h"

#include <bit>
#include <optional>
#include <string>

#include <boost/algorithm/string.hpp>

using namespace std;

namespace mmotd::platform {

string to_string(endian endian_value) {
    using namespace std::literals;
    if (endian_value == endian::big) {
        return "big-endian"s;
    } else if (endian_value == endian::little) {
        return "little-endian"s;
    } else if (endian_value != endian::big && endian_value != endian::little) {
        return "mixed-endian"s;
    } else {
        return "unknown-endian"s;
    }
}

optional<endian> from_endian_string(string endian_str) {
    using namespace boost::algorithm;
    if (icontains(endian_str, "big")) {
        return endian::big;
    } else if (icontains(endian_str, "little")) {
        return endian::little;
    } else {
        return std::nullopt;
    }
}

bool HardwareDetails::empty() const noexcept {
    return std::empty(machine_type) && std::empty(machine_model) && cpu_core_count == 0 && std::empty(cpu_name) &&
           !byte_order && std::empty(gpu_name) && std::empty(monitor_name) && std::empty(monitor_resolution);
}

} // namespace mmotd::platform
