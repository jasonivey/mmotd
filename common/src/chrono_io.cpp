#include "common/include/chrono_io.h"

#include <chrono>

#include <fmt/chrono.h>

using namespace std;

namespace mmotd::chrono::io {

string to_string(std::chrono::system_clock::time_point time_point, std::string chrono_format) {
    auto local_time = fmt::localtime(std::chrono::system_clock::to_time_t(time_point));
    auto time_point_str = fmt::format(chrono_format, local_time);
    auto am_index = time_point_str.rfind("AM");
    auto pm_index = time_point_str.rfind("PM");
    if (am_index != string::npos) {
        time_point_str[am_index] = 'a';
        time_point_str[am_index + 1] = 'm';
    } else if (pm_index != string::npos) {
        time_point_str[pm_index] = 'p';
        time_point_str[pm_index + 1] = 'm';
    }
    return time_point_str;
}

} // namespace mmotd::chrono::io
