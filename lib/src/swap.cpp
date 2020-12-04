// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/human_size.h"
#include "common/include/platform_error.h"
#include "lib/include/computer_information.h"
#include "lib/include/swap.h"

#include <unordered_map>

#include <fmt/format.h>
#include <plog/Log.h>
#include <scope_guard.hpp>

#include <sys/sysctl.h>

using fmt::format;
using namespace std;

bool gLinkSwapUsage = false;

namespace mmotd {

static const bool swap_usage_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::Swap>(); });

bool Swap::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        return GetSwapUsage();
    }
    return has_queried;
}

optional<mmotd::ComputerValues> Swap::GetInformation() const {
    return !details_.empty() ? make_optional(details_) : nullopt;
}

namespace detail {

optional<tuple<uint64_t, uint64_t, uint64_t, bool>> GetSwapUsageImpl() {
    using mmotd::algorithm::string::to_human_size;

    int mib[2] = {CTL_VM, VM_SWAPUSAGE};
    auto swap_usage = xsw_usage{};
    auto size = sizeof(xsw_usage);

    if (sysctl(mib, 2, &swap_usage, &size, NULL, 0) == -1) {
        auto error_str = string{"sysctl(VM_SWAPUSAGE) syscall failed"};
        if (errno != 0) {
            error_str += format(", details: {}", mmotd::platform::error::to_string(errno));
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

} // namespace detail

bool Swap::GetSwapUsage() {
    details_.clear();
    auto swap_usage_wrapper = detail::GetSwapUsageImpl();
    if (!swap_usage_wrapper) {
        return false;
    }

    bool encrypted = false;
    uint64_t total, available = 0;
    tie(total, std::ignore, available, encrypted) = *swap_usage_wrapper;
    auto percent_used = 0.0;
    if (total != 0) {
        percent_used = (static_cast<double>(total - available) / static_cast<double>(total)) * 100.0;
    }
    details_.push_back(
        make_tuple("swap usage", format("{:.01f}% ({})", percent_used, encrypted ? "encrypted" : "decrypted")));
    return !details_.empty();
}

} // namespace mmotd
