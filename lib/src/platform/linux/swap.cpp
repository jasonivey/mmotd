// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/human_size.h"
#include "common/include/posix_error.h"
#include "lib/include/platform/swap.h"

#include <cstdint>
#include <optional>
#include <string>
#include <tuple>

#include <fmt/format.h>
#include <plog/Log.h>

#include <sys/sysinfo.h>

using fmt::format;
using mmotd::algorithm::string::to_human_size;
using mmotd::platform::SwapDetails;
using namespace std;

namespace {

#if defined(ALWAYS_DISABLED_DOCUMENTATION_)
struct sysinfo {
    long uptime;                                  // Seconds since boot
    unsigned long loads[3];                       // 1, 5, and 15 minute load averages
    unsigned long totalram;                       // Total usable main memory size
    unsigned long freeram;                        // Available memory size
    unsigned long sharedram;                      // Amount of shared memory
    unsigned long bufferram;                      // Memory used by buffers
    unsigned long totalswap;                      // Total swap space size
    unsigned long freeswap;                       // swap space still available
    unsigned short procs;                         // Number of current processes
    unsigned long totalhigh;                      // Total high memory size
    unsigned long freehigh;                       // Available high memory size
    unsigned int mem_unit;                        // Memory unit size in bytes
    char _f[20 - 2 * sizeof(long) - sizeof(int)]; // Padding to 64 bytes
};
#endif

optional<SwapDetails> GetSwapMemoryUsage() {
    struct sysinfo info {};
    if (sysinfo(&info) == -1) {
        auto error_str = mmotd::error::posix_error::to_string();
        PLOG_ERROR << format(FMT_STRING("error calling sysinfo, {}"), error_str);
        return nullopt;
    }

    auto total = uint64_t{info.totalswap} * info.mem_unit;
    auto free = uint64_t{info.freeswap} * info.mem_unit;
    auto percent_used = 0.0;
    if (total != 0) {
        percent_used = (static_cast<double>(total - free) / static_cast<double>(total)) * 100.0;
    }

    auto swap_details = SwapDetails{total, free, percent_used, false};

    PLOG_VERBOSE << format(FMT_STRING("swap memory total: {}, {} bytes"),
                           to_human_size(swap_details.total),
                           swap_details.total);
    PLOG_VERBOSE << format(FMT_STRING("swap memory free: {}, {} bytes"),
                           to_human_size(swap_details.free),
                           swap_details.free);
    PLOG_VERBOSE << format(FMT_STRING("swap memory percent used: {:.02f}"), swap_details.percent_used);
    PLOG_VERBOSE << format(FMT_STRING("swap memory encrypted: {}"), swap_details.encrypted);

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
