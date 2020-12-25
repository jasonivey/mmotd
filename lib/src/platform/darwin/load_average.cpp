// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__APPLE__)

#include "lib/include/platform/load_average.h"

#include <ctime>
#include <optional>

#include <fmt/format.h>
#include <plog/Log.h>

#include <sys/sysctl.h>
#include <time.h>

using fmt::format;
using namespace std;

namespace {

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

optional<double> GetSystemLoadAverage() {
    auto load = loadavg{};
    auto load_size = sizeof(load);
    if (sysctlbyname("vm.loadavg", &load, &load_size, NULL, 0) == -1) {
        PLOG_ERROR << "sysctlbyname returned -1 calling vm.loadavg";
        return nullopt;
    } else {
        PLOG_INFO << format("sysctlbyname returned 1 min: {}, 5 min: {}, 15 min: {}",
                            load.ldavg[0] / static_cast<double>(load.fscale),
                            load.ldavg[1] / static_cast<double>(load.fscale),
                            load.ldavg[2] / static_cast<double>(load.fscale));
        return make_optional(load.ldavg[0] / static_cast<double>(load.fscale));
    }
}

}

namespace mmotd::platform {

LoadAverageDetails GetLoadAverageDetails() {
    auto details = LoadAverageDetails{};

    auto cpu_count_wrapper = detail::GetCpuCount();
    if (cpu_count_wrapper) {
        auto cpu_count = *cpu_count_wrapper;
        details.push_back(make_tuple("processor count", to_string(cpu_count)));
    }

    auto system_load_average_wrapper = GetSystemLoadAverage();
    if (system_load_average_wrapper) {
        auto system_load_average = *system_load_average_wrapper;
        details.push_back(make_tuple("load average", format("{:.02f}%", system_load_average)));
    }

    return details;
}

} // namespace mmotd::platform

#endif
