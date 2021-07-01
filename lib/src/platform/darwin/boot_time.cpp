// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__APPLE__)
#include "common/include/chrono_io.h"
#include "common/include/logging.h"
#include "common/include/posix_error.h"

#include <array>
#include <chrono>
#include <ctime>
#include <iterator>
#include <optional>
#include <string>

#include <fmt/format.h>

#include <sys/sysctl.h>

using fmt::format;
using namespace std;
using std::chrono::system_clock;

namespace mmotd::platform {

optional<std::chrono::system_clock::time_point> GetBootTime() {
    auto mib = array<int, 2>{CTL_KERN, KERN_BOOTTIME};
    auto result = timeval{};
    size_t result_len = sizeof(timeval);

    if (sysctl(data(mib), 2, &result, &result_len, nullptr, 0) == -1) {
        auto error_str = string{};
        if (auto posix_errno = mmotd::error::posix_error::to_string(); !posix_errno.empty()) {
            error_str += format(FMT_STRING(", details: {}"), posix_errno);
        }
        LOG_ERROR("sysctl(KERN_BOOTTIME) syscall failed{}", error_str);
        return nullopt;
    }

    auto boot_time_point = system_clock::from_time_t(result.tv_sec);
    LOG_VERBOSE("sysctl(KERN_BOOTTIME): {}", mmotd::chrono::io::to_string(boot_time_point, "%d-%h-%Y %I:%M:%S%p %Z"));
    return {boot_time_point};
}

} // namespace mmotd::platform
#endif
