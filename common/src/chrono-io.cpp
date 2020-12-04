#include "common/include/chrono-io.h"

#include <chrono>
#include <fmt/chrono.h>

using namespace std;
using fmt::format;
using fmt::to_string_view;

namespace mmotd::chrono::io {

string to_string(std::chrono::system_clock::time_point time_point, std::string chrono_format) {
    auto local_time = fmt::localtime(std::chrono::system_clock::to_time_t(time_point));
    return format(chrono_format, local_time);
}

} // namespace mmotd::chrono::io
