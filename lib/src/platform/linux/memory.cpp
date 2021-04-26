// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/human_size.h"
#include "common/include/information_definitions.h"
#include "common/include/information_objects.h"
#include "common/include/logging.h"
#include "common/include/posix_error.h"
#include "lib/include/platform/memory.h"

#include <cstdint>
#include <optional>
#include <string>
#include <tuple>

#include <fmt/format.h>

#include <sys/sysinfo.h>

using namespace std;
using fmt::format;
using mmotd::algorithm::string::to_human_size;

namespace {

optional<mmotd::platform::MemoryDetails> GetMemoryUsage() {
    struct sysinfo info {};
    if (sysinfo(&info) == -1) {
        auto error_str = mmotd::error::posix_error::to_string();
        LOG_ERROR("error calling sysinfo, {}", error_str);
        return nullopt;
    }

    auto total = uint64_t{info.totalram} * info.mem_unit;
    LOG_VERBOSE("memory total ram: {}, {} bytes", to_human_size(total), total);
    auto total_high = uint64_t{info.totalhigh} * info.mem_unit;
    LOG_VERBOSE("memory total high ram: {}, {} bytes", to_human_size(total_high), total_high);
    if (total_high != 0) {
        total = (total_high << 32) | total;
    }

    auto free = uint64_t{info.freeram} * info.mem_unit;
    LOG_VERBOSE("memory free ram: {}, {} bytes", to_human_size(free), free);
    auto free_high = uint64_t{info.freehigh} * info.mem_unit;
    LOG_VERBOSE("memory free high ram: {}, {} bytes", to_human_size(free_high), free_high);
    if (free_high != 0) {
        free = (free_high << 32) | free;
    }

    LOG_VERBOSE("memory total: {}, {} bytes", to_human_size(total), total);
    LOG_VERBOSE("memory free: {}, {} bytes", to_human_size(free), free);
    auto percent_used = 0.0;
    if (total != 0) {
        percent_used = (static_cast<double>(total - free) / static_cast<double>(total)) * 100.0;
        LOG_VERBOSE("percent used: {:.01f}", percent_used);
    }

    auto memory_details = mmotd::platform::MemoryDetails{};
    memory_details.total = total;
    memory_details.free = free;
    memory_details.percent_used = percent_used;

    return make_optional(memory_details);
}

} // namespace

namespace mmotd::platform {

MemoryDetails GetMemoryDetails() {
    if (auto memory_details_holder = GetMemoryUsage(); memory_details_holder) {
        return *memory_details_holder;
    } else {
        return MemoryDetails{};
    }
}

} // namespace mmotd::platform
