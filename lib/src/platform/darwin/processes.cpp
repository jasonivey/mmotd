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
    static constexpr int RETRY_COUNT = 8; // arbitrary retry count
    auto process_infos = vector<kinfo_proc>{};
    for (int i = 0; i != RETRY_COUNT && empty(process_infos); ++i) {
        LOG_VERBOSE("attempt #{} to get process list", i + 1);
        auto buffer_size = size_t{0};
        // get the size of the buffer
        if (sysctl(std::data(mib), 3, nullptr, &buffer_size, nullptr, 0) == -1) {
            LOG_ERROR("sysctl(KERN_PROC_ALL) failed, details: {}", mmotd::error::posix_error::to_string());
            return nullopt;
        } else {
            LOG_INFO("attempt #{}: buffer size: {}", i + 1, buffer_size);
        }

        // since time passes between these calls, double buffer size and add a little extra space for safety
        buffer_size += buffer_size + (buffer_size >> 3);
        LOG_INFO("attempt #{}: buffer size adjusted to: {}", i + 1, buffer_size);
        auto buffer = vector<uint8_t>(buffer_size, 0);
        if (sysctl(data(mib), 3, data(buffer), &buffer_size, nullptr, 0) == -1) {
            if (errno == ENOMEM) {
                LOG_ERROR("attempt #{} failed, sysctl(KERN_PROC_ALL) == ENOMEM, attempting larger allocation", i + 1);
                continue;
            }
            LOG_ERROR("attempt #{} failed, sysctl(KERN_PROC_ALL) failed, details: {}",
                      i + 1,
                      mmotd::error::posix_error::to_string());
            return nullopt;
        } else {
            const auto proc_count = buffer_size / sizeof(kinfo_proc);
            LOG_INFO("attempt #{}: buffer size now: {}, process count: {}", i + 1, buffer_size, proc_count);
        }

        // process the buffer
        const auto process_count = buffer_size / sizeof(kinfo_proc);
        process_infos.resize(process_count, kinfo_proc{});
        memcpy(data(process_infos), data(buffer), process_count * sizeof(kinfo_proc));
    }

    if (empty(process_infos)) {
        LOG_ERROR("failed to get process list");
        return nullopt;
    }

    auto pids = vector<int32_t>{};
    for (const auto &process_info : process_infos) {
        pids.push_back(process_info.kp_proc.p_pid);
    }

    return pids;
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
