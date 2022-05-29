// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__APPLE__)
#include "lib/include/platform/processes.h"

#include "common/include/logging.h"
#include "common/include/posix_error.h"

#include <array>
#include <cstdint>
#include <vector>

#include <fmt/format.h>

#include <sys/sysctl.h>

using fmt::format;
using namespace std;

namespace {

optional<size_t> GetProcessesInfoSize(int retry) {
    using namespace mmotd::error;
    auto mib = array<int, 3>{CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    auto buffer_size = size_t{0};
    if (sysctl(std::data(mib), 3, nullptr, &buffer_size, nullptr, 0) == -1) {
        LOG_ERROR("sysctl(KERN_PROC_ALL) failed, details: {}", posix_error::to_string());
        return nullopt;
    }
    // since time passes between these calls just double buffer size for safety
    LOG_INFO("getting process list, found buffer size: {}, adjusted: {}", buffer_size, buffer_size * (retry + 1));
    return buffer_size * (retry + 1);
}

bool GetProcessesInfoBuffer(size_t buffer_size, vector<uint8_t> &buffer) {
    using namespace mmotd::error;
    auto mib = array<int, 3>{CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    buffer.resize(buffer_size, 0);
    if (sysctl(data(mib), 3, data(buffer), &buffer_size, nullptr, 0) == -1) {
        if (errno == ENOMEM) {
            LOG_ERROR("getting process list, sysctl(KERN_PROC_ALL) == ENOMEM, attempting larger allocation");
            buffer.clear();
            return true;
        }
        LOG_ERROR("getting process list, sysctl(KERN_PROC_ALL) failed, details: {}", posix_error::to_string());
        return false;
    }
    const auto proc_count = buffer_size / sizeof(kinfo_proc);
    LOG_INFO("getting process list -- SUCCESS! buffer size: {}, process count: {}", buffer_size, proc_count);
    return true;
}

optional<vector<int32_t>> GetProcessesInfo() {
    static constexpr int RETRY_COUNT = 6; // arbitrary retry count 1-5
    auto process_infos = vector<kinfo_proc>{};
    for (int i = 1; i != RETRY_COUNT && empty(process_infos); ++i) {
        LOG_VERBOSE("getting process list #{}", i);
        auto buffer_size_wrapper = GetProcessesInfoSize(i);
        if (!buffer_size_wrapper) {
            return nullopt;
        }
        auto buffer_size = *buffer_size_wrapper;
        auto buffer = vector<uint8_t>{};
        if (!GetProcessesInfoBuffer(buffer_size, buffer)) {
            return nullopt;
        } else if (empty(buffer)) {
            // an empty buffer indicates that sysctl returned ENOMEM -- not enough memory
            continue;
        }
        // process the buffer
        const auto process_count = buffer_size / sizeof(kinfo_proc);
        process_infos.resize(process_count, kinfo_proc{});
        memcpy(data(process_infos), data(buffer), process_count * sizeof(kinfo_proc));
    }

    if (empty(process_infos)) {
        LOG_ERROR("getting process list -- process list empty");
        return nullopt;
    }

    auto pids = vector<int32_t>{};
    transform(begin(process_infos), end(process_infos), back_inserter(pids), [](const auto &process_info) {
        return process_info.kp_proc.p_pid;
    });

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
