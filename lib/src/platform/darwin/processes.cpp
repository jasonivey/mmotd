// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__APPLE__)
#include "common/include/logging.h"
#include "common/include/posix_error.h"
#include "lib/include/platform/processes.h"

#include <cstdint>
#include <vector>

#include <fmt/format.h>

#include <sys/sysctl.h>

using fmt::format;
using namespace std;

namespace {

optional<vector<int32_t>> GetProcessesInfo() {
    int mib[3] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    int count = 8; // arbitrary count
    for (int index = 0; index < count; ++index) {
        LOG_VERBOSE("{}. attempting to get process list", index);
        auto size = size_t{0};
        if (sysctl(mib, 3, nullptr, &size, nullptr, 0) == -1) {
            LOG_ERROR("sysctl(KERN_PROC_ALL) failed, details: {}", mmotd::error::posix_error::to_string());
            break;
        }

        size += size + (size >> 3); // add some
        auto buffer = vector<uint8_t>(size, 0);
        if (sysctl(mib, 3, buffer.data(), &size, nullptr, 0) == -1) {
            if (errno == ENOMEM) {
                LOG_ERROR("sysctl(KERN_PROC_ALL) failed with ENOMEM, attempting allocation again");
                continue;
            }
            LOG_ERROR("sysctl(KERN_PROC_ALL) failed, details: {}", mmotd::error::posix_error::to_string());
            break;
        }

        const auto *proc_list = reinterpret_cast<const kinfo_proc *>(buffer.data());
        auto proc_count = static_cast<size_t>(size / sizeof(kinfo_proc));
        LOG_INFO("sysctl(KERN_PROC_ALL) discovered {} processes", proc_count);
        if (proc_count == 0) {
            LOG_ERROR("sysctl(KERN_PROC_ALL) no PID's were returned");
            break;
        }

        auto pids = vector<int32_t>{};
        const kinfo_proc *current = proc_list;
        for (auto i = size_t{0}; i < proc_count && current != nullptr; ++i, ++current) {
            pids.push_back(current->kp_proc.p_pid);
        }
        return pids;
    }

    return nullopt;
}

} // namespace

namespace mmotd::platform {

optional<size_t> GetProcessCount() {
    auto process_ids = GetProcessesInfo();
    if (!process_ids) {
        return nullopt;
    }
    return make_optional((*process_ids).size());
}

} // namespace mmotd::platform
#endif
