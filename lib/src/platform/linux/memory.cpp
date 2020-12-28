// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/human_size.h"
#include "common/include/posix_error.h"
#include "lib/include/platform/memory.h"

#include <cstdint>
#include <optional>
#include <string>
#include <tuple>

#include <fmt/format.h>
#include <plog/Log.h>

#include <sys/sysinfo.h>

using namespace std;
using fmt::format;

namespace {

//struct sysinfo {
//    long uptime;             /* Seconds since boot */
//    unsigned long loads[3];  /* 1, 5, and 15 minute load averages */
//    unsigned long totalram;  /* Total usable main memory size */
//    unsigned long freeram;   /* Available memory size */
//    unsigned long sharedram; /* Amount of shared memory */
//    unsigned long bufferram; /* Memory used by buffers */
//    unsigned long totalswap; /* Total swap space size */
//    unsigned long freeswap;  /* swap space still available */
//    unsigned short procs;    /* Number of current processes */
//    unsigned long totalhigh; /* Total high memory size */
//    unsigned long freehigh;  /* Available high memory size */
//    unsigned int mem_unit;   /* Memory unit size in bytes */
//    char _f[20-2*sizeof(long)-sizeof(int)]; /* Padding to 64 bytes */
//};

optional<tuple<uint64_t, uint64_t>> GetMemoryUsage() {
    using mmotd::algorithm::string::to_human_size;

    struct sysinfo info {};
    if (sysinfo(&info) == -1) {
        auto error_str = mmotd::error::posix_error::to_string();
        PLOG_ERROR << format("error calling sysinfo, {}", error_str);
        return nullopt;
    }

    auto total = uint64_t{info.totalram} * info.mem_unit;
    PLOG_VERBOSE << format("memory total ram: {}, {} bytes", to_human_size(total), total);
    auto total_high = uint64_t{info.totalhigh} * info.mem_unit;
    PLOG_VERBOSE << format("memory total high ram: {}, {} bytes", to_human_size(total_high), total_high);
    if (total_high != 0) {
        total = (total_high << 32) | total;
    }

    auto free = uint64_t{info.freeram} * info.mem_unit;
    PLOG_VERBOSE << format("memory free ram: {}, {} bytes", to_human_size(free), free);
    auto free_high = uint64_t{info.freehigh} * info.mem_unit;
    PLOG_VERBOSE << format("memory free high ram: {}, {} bytes", to_human_size(free_high), free_high);
    if (free_high != 0) {
        free = (free_high << 32) | free;
    }

    PLOG_VERBOSE << format("memory total: {}, {} bytes", to_human_size(total), total);
    PLOG_VERBOSE << format("memory free: {}, {} bytes", to_human_size(free), free);

    return make_optional(make_tuple(total, free));
}

} // namespace

namespace mmotd::platform {

MemoryDetails GetMemoryDetails() {
    using mmotd::algorithm::string::to_human_size;

    auto usage_wrapper = GetMemoryUsage();
    if (!usage_wrapper) {
        return MemoryDetails{};
    }

    auto [total, free] = *usage_wrapper;
    auto percent_used = 0.0;
    if (total != 0) {
        percent_used = (static_cast<double>(total - free) / static_cast<double>(total)) * 100.0;
    }

    auto details = MemoryDetails{};
    details.push_back(make_tuple("memory usage", format("total: {}", to_human_size(total))));
    details.push_back(make_tuple("memory usage", format("percent: {:.02f}%", percent_used)));
    details.push_back(make_tuple("memory usage", format("free: {}", to_human_size(free))));
    return details;
}

} // namespace mmotd::platform
