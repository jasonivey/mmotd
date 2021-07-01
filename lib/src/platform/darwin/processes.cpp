// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__APPLE__)
#include "common/include/logging.h"
#include "common/include/posix_error.h"
#include "lib/include/platform/processes.h"

#include <array>
#include <cstdint>
#include <vector>

#include <fmt/format.h>

#include <sys/sysctl.h>

using fmt::format;
using namespace std;

namespace {

optional<vector<int32_t>> GetProcessesInfo() {
    auto mib = array<int, 3>{CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    static constexpr int count = 8; // arbitrary retry count
    for (int i = 0; i != count; ++i) {
        LOG_VERBOSE("attempt #{} to get process list", i + 1);
        auto size = size_t{0};
        if (sysctl(std::data(mib), 3, nullptr, &size, nullptr, 0) == -1) {
            LOG_ERROR("sysctl(KERN_PROC_ALL) failed, details: {}", mmotd::error::posix_error::to_string());
            break;
        }

        size += size + (size >> 3); // add some
        auto buffer = vector<uint8_t>(size, 0);
        if (sysctl(std::data(mib), 3, std::data(buffer), &size, nullptr, 0) == -1) {
            if (errno == ENOMEM) {
                LOG_ERROR("attempt #{} failed, sysctl(KERN_PROC_ALL) == ENOMEM, attempting larger allocation", i + 1);
                continue;
            }
            LOG_ERROR("attempt #{} failed, sysctl(KERN_PROC_ALL) failed, details: {}",
                      i + 1,
                      mmotd::error::posix_error::to_string());
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
        for (auto j = size_t{0}; j < proc_count && current != nullptr; ++j, ++current) {
            pids.push_back(current->kp_proc.p_pid);
        }
        return {pids};
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
    return {(*process_ids).size()};
}

} // namespace mmotd::platform
#endif
