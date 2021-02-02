// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/human_size.h"
#include "common/include/information_definitions.h"
#include "common/include/information_objects.h"
#include "common/include/posix_error.h"
#include "lib/include/platform/memory.h"

#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <fmt/format.h>
#include <plog/Log.h>
#include <scope_guard.hpp>

#include <mach/mach.h>
#include <mach/mach_types.h>
#include <mach/vm_statistics.h>
#include <sys/sysctl.h>

using fmt::format;
using namespace std;

namespace mmotd::platform {

namespace {

bool GetVmStat(vm_statistics_data_t *vmstat) {
    mach_msg_type_number_t count = sizeof(*vmstat) / sizeof(integer_t);
    auto mach_port = mach_host_self();
    // auto deallocate the mach port
    auto mach_port_deallocator =
        sg::make_scope_guard([&mach_port]() { mach_port_deallocate(mach_task_self(), mach_port); });

    auto retval = host_statistics(mach_port, HOST_VM_INFO, reinterpret_cast<host_info_t>(vmstat), &count);
    if (retval != KERN_SUCCESS) {
        PLOG_ERROR << format("when calling host_statistics, details: {}", mach_error_string(retval));
        return false;
    }

    return true;
}

optional<tuple<uint64_t, uint64_t, double, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t>> GetMemoryUsageImpl() {
    using mmotd::algorithm::string::to_human_size;

    int mib[2] = {CTL_HW, HW_MEMSIZE};
    auto total = uint64_t{0};
    auto len = sizeof(uint64_t);
    if (sysctl(mib, 2, &total, &len, NULL, 0) == -1) {
        auto error_str = string{"sysctl(HW_MEMSIZE) syscall failed"};
        if (auto errno_str = mmotd::error::posix_error::to_string(); !errno_str.empty()) {
            error_str += format(", details: {}", errno_str);
        }
        PLOG_ERROR << error_str;
        return nullopt;
    }

    auto vm_statistics_data = vm_statistics_data_t{};
    if (!GetVmStat(&vm_statistics_data)) {
        return nullopt;
    }

    int pagesize = getpagesize();
    PLOG_VERBOSE << format("pagesize: {}", pagesize);
    PLOG_VERBOSE << format("active count: {}, {}",
                           to_human_size(vm_statistics_data.active_count),
                           vm_statistics_data.active_count);
    PLOG_VERBOSE << format("inactive count: {}, {}",
                           to_human_size(vm_statistics_data.inactive_count),
                           vm_statistics_data.inactive_count);
    PLOG_VERBOSE << format("wire count: {}, {}",
                           to_human_size(vm_statistics_data.wire_count),
                           vm_statistics_data.wire_count);
    PLOG_VERBOSE << format("free count: {}, {}",
                           to_human_size(vm_statistics_data.free_count),
                           vm_statistics_data.free_count);
    PLOG_VERBOSE << format("speculative count: {}, {}",
                           to_human_size(vm_statistics_data.speculative_count),
                           vm_statistics_data.speculative_count);

    auto active = static_cast<uint64_t>(vm_statistics_data.active_count) * pagesize;
    auto inactive = static_cast<uint64_t>(vm_statistics_data.inactive_count) * pagesize;
    auto wired = static_cast<uint64_t>(vm_statistics_data.wire_count) * pagesize;
    auto free = static_cast<uint64_t>(vm_statistics_data.free_count) * pagesize;
    auto speculative = static_cast<uint64_t>(vm_statistics_data.speculative_count) * pagesize;
    PLOG_VERBOSE << format("active: {}, {}", to_human_size(active), active);
    PLOG_VERBOSE << format("inactive: {}, {}", to_human_size(inactive), inactive);
    PLOG_VERBOSE << format("wired: {}, {}", to_human_size(wired), wired);
    PLOG_VERBOSE << format("free: {}, {}", to_human_size(free), free);
    PLOG_VERBOSE << format("speculative: {}, {}", to_human_size(speculative), speculative);

    auto avail = inactive + free;
    auto used = active + wired;
    free = speculative < free ? free - speculative : 0;
    PLOG_VERBOSE << format("avail = inactive + free: {}, {}", to_human_size(avail), avail);
    PLOG_VERBOSE << format("used = active + wired: {}, {}", to_human_size(used), used);
    PLOG_VERBOSE << format("free = free - speculative: {}, {}", to_human_size(free), free);

    auto percent = (static_cast<double>(total - avail) / static_cast<double>(total)) * 100.0;
    PLOG_VERBOSE << format("percent: {:.01f}", percent);

    return make_tuple(total, avail, percent, used, free, active, inactive, wired);
}

} // namespace

Details GetMemoryDetails() {
    using mmotd::algorithm::string::to_human_size;

    if (auto memory_usage_wrapper = GetMemoryUsageImpl(); !memory_usage_wrapper) {
        return Details{};
    } else {
        const auto [total, avail, percent, used, free, active, inactive, wired] = *memory_usage_wrapper;

        auto details = Details{};
        details.push_back(make_tuple("total", format("{}", to_human_size(total))));
        details.push_back(make_tuple("avail", format(": {}", to_human_size(avail))));
        details.push_back(make_tuple("percent", format("{:.02f}% of {}", percent, to_human_size(total))));
        details.push_back(make_tuple("used", format("{}", to_human_size(used))));
        details.push_back(make_tuple("free", format("{}", to_human_size(free))));
        details.push_back(make_tuple("active", format("{}", to_human_size(active))));
        details.push_back(make_tuple("inactive", format("{}", to_human_size(inactive))));
        details.push_back(make_tuple("wired", format("{}", to_human_size(wired))));
        return details;
    }
}

} // namespace mmotd::platform
