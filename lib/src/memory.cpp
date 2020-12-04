// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/human_size.h"
#include "common/include/platform_error.h"
#include "lib/include/computer_information.h"
#include "lib/include/memory.h"

#include <fmt/format.h>
#include <plog/Log.h>
#include <scope_guard.hpp>

#include <mach/mach.h>
#include <mach/mach_types.h>
#include <mach/vm_statistics.h>
#include <sys/sysctl.h>

using fmt::format;
using namespace std;

bool gLinkMemoryUsage = false;

namespace mmotd {

static const bool memory_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::Memory>(); });

bool Memory::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        return GetMemoryUsage();
    }
    return has_queried;
}

optional<mmotd::ComputerValues> Memory::GetInformation() const {
    return !details_.empty() ? make_optional(details_) : nullopt;
}

namespace detail {

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
        if (errno != 0) {
            error_str += format(", details: {}", mmotd::platform::error::to_string(errno));
        }
        PLOG_ERROR << error_str;
        return nullopt;
    }

    auto vm_statistics_data = vm_statistics_data_t{};
    if (!detail::GetVmStat(&vm_statistics_data)) {
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

} // namespace detail

bool Memory::GetMemoryUsage() {
    using mmotd::algorithm::string::to_human_size;

    auto memory_usage_wrapper = detail::GetMemoryUsageImpl();
    if (!memory_usage_wrapper) {
        return false;
    }
    uint64_t total, avail, used, free, active, inactive, wired = 0;
    double percent = 0.0;
    tie(total, avail, percent, used, free, active, inactive, wired) = (*memory_usage_wrapper);
    details_.push_back(make_tuple("memory usage", format("total: {} {}", to_human_size(total), total)));
    details_.push_back(make_tuple("memory usage", format("avail: {} {}", to_human_size(avail), avail)));
    details_.push_back(make_tuple("memory usage", format("percent: {:.02f}%", percent)));
    details_.push_back(make_tuple("memory usage", format("used: {} {}", to_human_size(used), used)));
    details_.push_back(make_tuple("memory usage", format("free: {} {}", to_human_size(free), free)));
    details_.push_back(make_tuple("memory usage", format("active: {} {}", to_human_size(active), active)));
    details_.push_back(make_tuple("memory usage", format("inactive: {} {}", to_human_size(inactive), inactive)));
    details_.push_back(make_tuple("memory usage", format("wired: {} {}", to_human_size(wired), wired)));
    return true;
}

} // namespace mmotd