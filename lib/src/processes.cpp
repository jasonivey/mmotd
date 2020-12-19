// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
// #include "common/include/chrono_io.h"
#include "common/include/posix_error.h"
#include "lib/include/computer_information.h"
#include "lib/include/processes.h"

#include <cstdint>
#include <vector>

#include <fmt/format.h>
#include <plog/Log.h>

#include <sys/sysctl.h>

using fmt::format;
using namespace std;

bool gLinkProcessesInfo = false;

namespace mmotd {

static const bool processes_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::Processes>(); });

bool Processes::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        return GetProcessCount();
    }
    return has_queried;
}

namespace detail {

vector<int32_t> GetProcessesInfo() {
    int mib[3] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    int count = 8; // arbitrary count
    for (int index = 0; index < count; ++index) {
        PLOG_VERBOSE << format("{}. attempting to get process list", index);
        auto size = size_t{0};
        if (sysctl(mib, 3, NULL, &size, NULL, 0) == -1) {
            PLOG_ERROR << format("sysctl(KERN_PROC_ALL) failed, details: {}", mmotd::error::posix_error::to_string());
            break;
        }

        size += size + (size >> 3); // add some
        auto buffer = vector<uint8_t>(size, 0);
        if (sysctl(mib, 3, buffer.data(), &size, NULL, 0) == -1) {
            if (errno == ENOMEM) {
                PLOG_ERROR << format("sysctl(KERN_PROC_ALL) failed with ENOMEM, attempting allocation again");
                continue;
            }
            PLOG_ERROR << format("sysctl(KERN_PROC_ALL) failed, details: {}", mmotd::error::posix_error::to_string());
            break;
        }

        const kinfo_proc *proc_list = reinterpret_cast<const kinfo_proc *>(buffer.data());
        auto proc_count = static_cast<size_t>(size / sizeof(kinfo_proc));
        PLOG_INFO << format("sysctl(KERN_PROC_ALL) discovered {} processes", proc_count);
        if (proc_count == 0) {
            PLOG_ERROR << format("sysctl(KERN_PROC_ALL) no PID's were returned");
            break;
        }

        auto pids = vector<int32_t>{};
        const kinfo_proc *current = proc_list;
        for (auto i = size_t{0}; i < proc_count && current != nullptr; ++i, ++current) {
            pids.push_back(current->kp_proc.p_pid);
        }
        return pids;
    }

    return vector<int32_t>{};
}

} // namespace detail
std::optional<mmotd::ComputerValues> Processes::GetInformation() const {
    return details_.empty() ? nullopt : make_optional(details_);
}

bool Processes::GetProcessCount() {
    auto process_ids = detail::GetProcessesInfo();
    auto process_ids_str = string{};
    //for (const auto id : process_ids) {
    //    process_ids_str += format("{}{}", process_ids_str.empty() ? "" : ", ", id);
    //}
    PLOG_VERBOSE << format("found process ids: {}", process_ids_str);
    details_.push_back(make_tuple("processes", format("ids: {}", process_ids_str)));
    details_.push_back(make_tuple("processes", format("count: {}", process_ids.size())));
    return true;
}

} // namespace mmotd
