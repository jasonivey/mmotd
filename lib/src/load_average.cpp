// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/platform_error.h"
#include "lib/include/computer_information.h"
#include "lib/include/load_average.h"

// #include <mach/host_info.h>
// #include <mach/mach.h>
// #include <mach/mach_error.h>
// #include <mach/mach_host.h>
// #include <mach/mach_init.h>
// #include <mach/mach_types.h>
// #include <mach/processor_info.h>
// #include <mach/vm_map.h>
// #include <mach/vm_statistics.h>
#include <sys/sysctl.h>
// #include <sys/types.h>
#include <time.h>

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkLoadAverage = false;

namespace mmotd {

static const bool load_average_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::LoadAverage>(); });

bool LoadAverage::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        return GetLoadAverage();
    }
    return has_queried;
}

optional<mmotd::ComputerValues> LoadAverage::GetInformation() const {
    if (!details_.empty()) {
        return make_optional(details_);
    } else {
        return nullopt;
    }
}

namespace detail {

optional<int32_t> GetCpuCount() {
    auto cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpu_count == -1) {
        PLOG_ERROR << "sysconf returned -1 calling get processor count _SC_NPROCESSORS_ONLN";
        return nullopt;
    } else {
        PLOG_INFO << format("sysconf for _SC_NPROCESSORS_ONLN returned {} processors", cpu_count);
        return make_optional(cpu_count);
    }
}

#if 0
void LoadAverageSleep(time_t seconds) {
    struct timespec request_time_spec = {seconds, 0};
    struct timespec interrupted_time_spec = {0, 0};
    if (nanosleep(&request_time_spec, &interrupted_time_spec) == -1) {
        PLOG_ERROR << format("nanosleep was interrupted and returned {}s, {}ns early, error: {}",
                             interrupted_time_spec.tv_sec,
                             interrupted_time_spec.tv_nsec,
                             mmotd::platform::error::to_string(errno));
    }
}

optional<host_cpu_load_info_data_t> GetCpuLoadInfoData() {
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    mach_port_t mach_port = mach_host_self();
    host_cpu_load_info_data_t cpu_load_info_data = {0};
    kern_return_t error =
        host_statistics(mach_port, HOST_CPU_LOAD_INFO, reinterpret_cast<host_info_t>(&cpu_load_info_data), &count);
    if (error != KERN_SUCCESS) {
        PLOG_ERROR << format("host_statistics returned error {}", error);
        return nullopt;
    }

    return make_optional(cpu_load_info_data);
}

optional<double> GetCpuUsage(time_t delay_seconds) {
    auto cpu_load_info_data_wrapper1 = GetCpuLoadInfoData();
    if (!cpu_load_info_data_wrapper1) {
        return nullopt;
    }
    LoadAverageSleep(delay_seconds);
    auto cpu_load_info_data_wrapper2 = GetCpuLoadInfoData();
    if (!cpu_load_info_data_wrapper2) {
        return nullopt;
    }

    auto cpu_load_info_data1 = cpu_load_info_data_wrapper1.value();
    auto cpu_load_info_data2 = cpu_load_info_data_wrapper2.value();

    auto current_user = cpu_load_info_data1.cpu_ticks[CPU_STATE_USER];
    auto current_system = cpu_load_info_data1.cpu_ticks[CPU_STATE_SYSTEM];
    auto current_nice = cpu_load_info_data1.cpu_ticks[CPU_STATE_NICE];
    auto current_idle = cpu_load_info_data1.cpu_ticks[CPU_STATE_IDLE];

    auto next_user = cpu_load_info_data2.cpu_ticks[CPU_STATE_USER];
    auto next_system = cpu_load_info_data2.cpu_ticks[CPU_STATE_SYSTEM];
    auto next_nice = cpu_load_info_data2.cpu_ticks[CPU_STATE_NICE];
    auto next_idle = cpu_load_info_data2.cpu_ticks[CPU_STATE_IDLE];

    auto diff_user = next_user - current_user;
    auto diff_system = next_system - current_system;
    auto diff_nice = next_nice - current_nice;
    auto diff_idle = next_idle - current_idle;

    auto cpu_usage = static_cast<double>(diff_user + diff_system + diff_nice) /
                     static_cast<double>(diff_user + diff_system + diff_nice + diff_idle) * 100.0;
    return make_optional(cpu_usage);
}
#endif

optional<double> GetSystemLoadAverage() {
    auto load = loadavg{};
    auto load_size = sizeof(load);
    if (sysctlbyname("vm.loadavg", &load, &load_size, NULL, 0) == -1) {
        PLOG_ERROR << "sysctlbyname returned -1 calling vm.loadavg";
        return false;
    } else {
        PLOG_INFO << format("sysctlbyname returned 1 min: {}, 5 min: {}, 15 min: {}",
                            load.ldavg[0] / static_cast<double>(load.fscale),
                            load.ldavg[1] / static_cast<double>(load.fscale),
                            load.ldavg[2] / static_cast<double>(load.fscale));
        return make_optional(load.ldavg[0] / static_cast<double>(load.fscale));
    }
}

} // namespace detail

bool LoadAverage::GetLoadAverage() {
    // auto cpu_usage_wrapper = detail::GetCpuUsage(1);
    // if (!cpu_usage_wrapper) {
    //     return false;
    // }
    // auto cpu_usage = cpu_usage_wrapper.value();
    auto system_load_average_wrapper = detail::GetSystemLoadAverage();
    if (!system_load_average_wrapper) {
        return false;
    }
    auto system_load_average = system_load_average_wrapper.value();
    auto cpu_count_wrapper = detail::GetCpuCount();
    if (!cpu_count_wrapper) {
        return false;
    }
    auto cpu_count = cpu_count_wrapper.value();
    //auto load_percent = static_cast<double>(cpu_count) / load_average;
    // details_.push_back(make_tuple("load average", format("cpu usage: {:.02f}%", cpu_usage)));
    details_.push_back(make_tuple("load average", format("system load average: {:.02f}%", system_load_average)));
    details_.push_back(make_tuple("processor count", to_string(cpu_count)));
    return true;
}

} // namespace mmotd
