// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__linux__)
#include "common/include/chrono_io.h"
#include "common/include/iostream_error.h"
#include "common/include/logging.h"

#include <chrono>
#include <fstream>
#include <optional>
#include <string>

#include <fmt/format.h>

using namespace std;
using fmt::format;

constexpr static const char *UPTIME_FILENAME = "/proc/uptime";

namespace mmotd::platform {

optional<std::chrono::system_clock::time_point> GetBootTime() {
    auto time_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    auto uptime_file = ifstream{};
    uptime_file.exceptions(std::ifstream::goodbit);
    uptime_file.open(UPTIME_FILENAME, ios_base::in);

    if (!uptime_file.is_open() || uptime_file.fail() || uptime_file.bad()) {
        LOG_ERROR("unable to open {} for reading, {}",
                  UPTIME_FILENAME,
                  mmotd::error::ios_flags::to_string(uptime_file));
        return make_optional(std::chrono::system_clock::now());
    }
    auto uptime = double{};
    uptime_file >> uptime;

    auto boot_time_point = std::chrono::system_clock::from_time_t(time_now - static_cast<time_t>(uptime));
    return make_optional(boot_time_point);
}

} // namespace mmotd::platform
#endif
