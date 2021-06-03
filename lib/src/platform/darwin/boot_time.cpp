// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__APPLE__)
#include "common/include/chrono_io.h"
#include "common/include/logging.h"
#include "common/include/posix_error.h"

#include <chrono>
#include <ctime>
#include <iterator>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <fmt/format.h>

#include <sys/sysctl.h>

using fmt::format;
using mmotd::chrono::io::to_string;
using namespace std;

namespace mmotd::platform {

optional<std::chrono::system_clock::time_point> GetBootTime() {
    int mib[2] = {CTL_KERN, KERN_BOOTTIME};
    auto result = timeval{};
    size_t result_len = sizeof(timeval);

    if (sysctl(mib, 2, &result, &result_len, nullptr, 0) == -1) {
        auto error_str = string{};
        if (auto errno_str = mmotd::error::posix_error::to_string(); !errno_str.empty()) {
            error_str = format(FMT_STRING(", details: {}"), errno_str);
        }
        LOG_ERROR("sysctl(KERN_BOOTTIME) syscall failed{}", error_str);
        return make_optional(std::chrono::system_clock::now());
    }

    auto boot_time_point = std::chrono::system_clock::from_time_t(result.tv_sec);
    LOG_VERBOSE("sysctl(KERN_BOOTTIME): {}", to_string(boot_time_point, "%d-%h-%Y %I:%M:%S%p %Z"));
    return make_optional(boot_time_point);
}

} // namespace mmotd::platform
#endif
