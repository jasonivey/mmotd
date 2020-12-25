// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include "common/include/chrono_io.h"
#include "common/include/posix_error.h"

#include <chrono>
#include <ctime>
#include <iterator>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

namespace mmotd::platform {

optional<string> GetBootTime() {
    //int mib[2] = {CTL_KERN, KERN_BOOTTIME};
    //auto result = timeval{};
    //size_t result_len = sizeof(timeval);

    //if (sysctl(mib, 2, &result, &result_len, NULL, 0) == -1) {
    //    auto error_str = string{"sysctl(KERN_BOOTTIME) syscall failed"};
    //    if (auto errno_str = mmotd::error::posix_error::to_string(); !errno_str.empty()) {
    //        error_str += format(", details: {}", errno_str);
    //    }
    //    PLOG_ERROR << error_str;
    //    return nullopt;
    //}
    //auto boot_time_point = std::chrono::system_clock::from_time_t(result.tv_sec);
    // Fri, 04-Dec-2020 07:49:36am MST
    // 04-Dec-2020 09:06:02AM MST
    //return make_optional(mmotd::chrono::io::to_string(boot_time_point, "{:%a, %d-%h-%Y %I:%M:%S%p %Z}"));
    return nullopt;
}

} // namespace mmotd::platform

#endif
