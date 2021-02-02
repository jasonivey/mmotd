// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/human_size.h"
#include "common/include/posix_error.h"
#include "lib/include/platform/swap.h"

#include <unordered_map>

#include <fmt/format.h>
#include <plog/Log.h>
#include <scope_guard.hpp>

#include <sys/sysctl.h>

using fmt::format;
using namespace std;

namespace {

optional<tuple<uint64_t, uint64_t, uint64_t, bool>> GetSwapUsage() {
    using mmotd::algorithm::string::to_human_size;

    int mib[2] = {CTL_VM, VM_SWAPUSAGE};
    auto swap_usage = xsw_usage{};
    auto size = sizeof(xsw_usage);

    if (sysctl(mib, 2, &swap_usage, &size, NULL, 0) == -1) {
        auto error_str = string{"sysctl(VM_SWAPUSAGE) syscall failed"};
        if (auto errno_str = mmotd::error::posix_error::to_string(); !errno_str.empty()) {
            error_str += format(", details: {}", errno_str);
        }
        PLOG_ERROR << error_str;
        return nullopt;
    }
    PLOG_VERBOSE << format("swap memory total: {}, {} bytes",
                           to_human_size(swap_usage.xsu_total),
                           swap_usage.xsu_total);
    PLOG_VERBOSE << format("swap memory used: {}, {} bytes", to_human_size(swap_usage.xsu_used), swap_usage.xsu_used);
    PLOG_VERBOSE << format("swap memory avail: {}, {} bytes",
                           to_human_size(swap_usage.xsu_avail),
                           swap_usage.xsu_avail);
    PLOG_VERBOSE << format("swap memory encrypted: {}", swap_usage.xsu_encrypted != 0 ? "true" : "false");

    return make_optional(
        make_tuple(swap_usage.xsu_total, swap_usage.xsu_used, swap_usage.xsu_avail, swap_usage.xsu_encrypted != 0));
}

} // namespace

namespace mmotd::platform {

SwapDetails GetSwapDetails() {
    using mmotd::algorithm::string::to_human_size;

    auto swap_usage_wrapper = GetSwapUsage();
    if (!swap_usage_wrapper) {
        return SwapDetails{};
    }

    auto [total, used, available, encrypted] = *swap_usage_wrapper;
    auto percent_used = 0.0;
    if (total != 0) {
        percent_used = (static_cast<double>(total - available) / static_cast<double>(total)) * 100.0;
    }

    auto details = SwapDetails{};
    auto percent_used_str = format("{:.01f}%", percent_used);
    if (!encrypted) {
        percent_used_str += format(" {}", to_human_size(total));
        details.push_back(make_tuple("total", format("{}", to_human_size(total))));
        details.push_back(make_tuple("free", format("{}", to_human_size(available))));
    }
    percent_used_str += format(" ({})", encrypted ? "encrypted" : "decrypted");
    details.push_back(make_tuple("percent", percent_used_str));

    return details;
}

} // namespace mmotd::platform
