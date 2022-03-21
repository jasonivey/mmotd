// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#if defined(__APPLE__)
#include "common/include/human_size.h"
#include "common/include/information_definitions.h"
#include "common/include/logging.h"
#include "common/include/posix_error.h"
#include "lib/include/platform/memory.h"

#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <fmt/format.h>
#include <scope_guard.hpp>

#include <mach/mach.h>
#include <mach/mach_types.h>
#include <mach/vm_statistics.h>
#include <sys/sysctl.h>
#include <unistd.h>

using fmt::format;
using namespace std;
using mmotd::algorithm::string::to_human_size;

namespace {

bool GetVmStat(vm_statistics64_t vmstat) {
    PRECONDITIONS(vmstat != nullptr, "vmstat is null");
    auto mach_port = mach_host_self();
    // auto deallocate the mach port
    auto mach_port_deallocator =
        sg::make_scope_guard([&mach_port]() noexcept { mach_port_deallocate(mach_task_self(), mach_port); });

    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    auto buffer = array<integer_t, HOST_VM_INFO64_COUNT>{};
    auto retval = host_statistics64(mach_port, HOST_VM_INFO64, data(buffer), &count);
    if (retval != KERN_SUCCESS) {
        LOG_ERROR("when calling host_statistics64, details: {}", mach_error_string(retval));
        return false;
    }

    static_assert(sizeof(vm_statistics64_data_t) == size(buffer) * sizeof(integer_t),
                  "vm_statistics64 size is not equal to data size");
    memcpy(vmstat, data(buffer), size(buffer));
    return true;
}

optional<uint64_t> GetTotalMemory() {
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    auto total = uint64_t{0};
    auto len = sizeof(uint64_t);
    if (sysctl(mib, 2, &total, &len, nullptr, 0) == -1) {
        auto error_str = string{};
        if (auto errno_str = mmotd::error::posix_error::to_string(); !errno_str.empty()) {
            error_str = format(FMT_STRING(", details: {}"), errno_str);
        }
        LOG_ERROR("sysctl(HW_MEMSIZE) syscall failed{}", error_str);
        return nullopt;
    }
    return make_optional(total);
}

optional<mmotd::platform::MemoryDetails> GetMemoryUsage() {
    auto total_memory_holder = GetTotalMemory();
    if (!total_memory_holder) {
        return nullopt;
    }
    auto total = *total_memory_holder;

    auto vm_statistics = vm_statistics64_data_t{};
    if (!GetVmStat(&vm_statistics)) {
        return nullopt;
    }

    const auto pagesize = getpagesize();

    LOG_VERBOSE("pagesize: {}", pagesize);
    LOG_VERBOSE("active count: {}, {}", to_human_size(vm_statistics.active_count), vm_statistics.active_count);
    LOG_VERBOSE("inactive count: {}, {}", to_human_size(vm_statistics.inactive_count), vm_statistics.inactive_count);
    LOG_VERBOSE("wire count: {}, {}", to_human_size(vm_statistics.wire_count), vm_statistics.wire_count);
    LOG_VERBOSE("free count: {}, {}", to_human_size(vm_statistics.free_count), vm_statistics.free_count);
    LOG_VERBOSE("speculative count: {}, {}",
                to_human_size(vm_statistics.speculative_count),
                vm_statistics.speculative_count);

    auto active = static_cast<uint64_t>(vm_statistics.active_count) * pagesize;
    auto inactive = static_cast<uint64_t>(vm_statistics.inactive_count) * pagesize;
    auto wired = static_cast<uint64_t>(vm_statistics.wire_count) * pagesize;
    auto free = static_cast<uint64_t>(vm_statistics.free_count) * pagesize;
    auto speculative = static_cast<uint64_t>(vm_statistics.speculative_count) * pagesize;
    LOG_VERBOSE("active: {}, {}", to_human_size(active), active);
    LOG_VERBOSE("inactive: {}, {}", to_human_size(inactive), inactive);
    LOG_VERBOSE("wired: {}, {}", to_human_size(wired), wired);
    LOG_VERBOSE("free: {}, {}", to_human_size(free), free);
    LOG_VERBOSE("speculative: {}, {}", to_human_size(speculative), speculative);

    auto avail = inactive + free;
    auto used = active + wired;
    free = speculative < free ? free - speculative : 0;
    LOG_VERBOSE("avail = inactive + free: {}, {}", to_human_size(avail), avail);
    LOG_VERBOSE("used = active + wired: {}, {}", to_human_size(used), used);
    LOG_VERBOSE("free = free - speculative: {}, {}", to_human_size(free), free);

    auto percent = (static_cast<double>(total - avail) / static_cast<double>(total)) * 100.0;
    LOG_VERBOSE("percent used: {:.01f}", percent);

    return make_optional(mmotd::platform::MemoryDetails{total, avail, percent, used, free, active, inactive, wired});
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
#endif
