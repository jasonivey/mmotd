// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/chrono_io.h"
#include "common/include/posix_error.h"
#include "lib/include/boot_time.h"
#include "lib/include/computer_information.h"

#include <chrono>
#include <ctime>
#include <iterator>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <fmt/format.h>
#include <plog/Log.h>

#include <sys/sysctl.h>

using fmt::format;
using namespace std;

bool gLinkBootTime = false;

namespace mmotd {

static const bool boot_time_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::BootTime>(); });

bool BootTime::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        return GetBootTime();
    }
    return has_queried;
}

optional<mmotd::ComputerValues> BootTime::GetInformation() const {
    return !details_.empty() ? make_optional(details_) : nullopt;
}

namespace detail {

optional<string> GetBootTimeImpl() {
    int mib[2] = {CTL_KERN, KERN_BOOTTIME};
    auto result = timeval{};
    size_t result_len = sizeof(timeval);

    if (sysctl(mib, 2, &result, &result_len, NULL, 0) == -1) {
        auto error_str = string{"sysctl(KERN_BOOTTIME) syscall failed"};
        if (auto errno_str = mmotd::error::posix_error::to_string(); !errno_str.empty()) {
            error_str += format(", details: {}", errno_str);
        }
        PLOG_ERROR << error_str;
        return nullopt;
    }
    auto boot_time_point = std::chrono::system_clock::from_time_t(result.tv_sec);
    // Fri, 04-Dec-2020 07:49:36am MST
    // 04-Dec-2020 09:06:02AM MST
    return make_optional(mmotd::chrono::io::to_string(boot_time_point, "{:%a, %d-%h-%Y %I:%M:%S%p %Z}"));
}

} // namespace detail

bool BootTime::GetBootTime() {
    details_.clear();
    auto random_boot_time_wrapper = detail::GetBootTimeImpl();
    if (!random_boot_time_wrapper) {
        return false;
    }

    auto boot_time = *random_boot_time_wrapper;
    details_.push_back(make_tuple("boot time", boot_time));
    return !details_.empty();
}

} // namespace mmotd
