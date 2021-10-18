// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__APPLE__)
#include "common/include/human_size.h"
#include "common/include/logging.h"
#include "common/include/posix_error.h"
#include "lib/include/platform/swap.h"

#include <optional>
#include <unordered_map>

#include <fmt/format.h>

#include <sys/sysctl.h>

using fmt::format;
using mmotd::algorithm::string::to_human_size;
using mmotd::platform::SwapDetails;
using namespace std;

namespace {

optional<SwapDetails> GetSwapMemoryUsage() {
    int mib[2] = {CTL_VM, VM_SWAPUSAGE};
    auto swap_usage = xsw_usage{};
    auto size = sizeof(xsw_usage);

    if (sysctl(mib, 2, &swap_usage, &size, nullptr, 0) == -1) {
        auto error_str = string{};
        if (auto errno_str = mmotd::error::posix_error::to_string(); !errno_str.empty()) {
            error_str = format(FMT_STRING(", details: {}"), errno_str);
        }
        LOG_ERROR("sysctl(VM_SWAPUSAGE) syscall failed{}", error_str);
        return nullopt;
    }

    auto percent_used = 0.0;
    if (swap_usage.xsu_total != 0) {
        percent_used = (static_cast<double>(swap_usage.xsu_total - swap_usage.xsu_avail) /
                        static_cast<double>(swap_usage.xsu_total)) *
                       100.0;
    }

    auto swap_details =
        SwapDetails{swap_usage.xsu_total, swap_usage.xsu_avail, percent_used, swap_usage.xsu_encrypted != 0};

    LOG_VERBOSE("swap memory total: {}, {} bytes", to_human_size(swap_details.total), swap_details.total);
    LOG_VERBOSE("swap memory free: {}, {} bytes", to_human_size(swap_details.free), swap_details.free);
    LOG_VERBOSE("swap memory percent used: {:.02f}", swap_details.percent_used);
    LOG_VERBOSE("swap memory encrypted: {}", swap_details.encrypted);

    return make_optional(swap_details);
}

} // namespace

namespace mmotd::platform {

SwapDetails GetSwapDetails() {
    if (auto swap_details_holder = GetSwapMemoryUsage(); swap_details_holder) {
        return *swap_details_holder;
    } else {
        return SwapDetails{};
    }
}

} // namespace mmotd::platform
#endif
