// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__APPLE__)
#include "common/include/logging.h"
#include "lib/include/platform/load_average.h"

#include <ctime>
#include <optional>

#include <fmt/format.h>

#include <sys/sysctl.h>
#include <unistd.h>

using fmt::format;
using namespace std;

namespace {

optional<int32_t> GetCpuCount() {
    auto cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpu_count == -1) {
        LOG_ERROR("sysconf returned -1 calling get processor count _SC_NPROCESSORS_ONLN");
        return nullopt;
    } else {
        LOG_INFO("sysconf for _SC_NPROCESSORS_ONLN returned {} processors", cpu_count);
        return make_optional(cpu_count);
    }
}

optional<double> GetSystemLoadAverage() {
    auto load = loadavg{};
    auto load_size = sizeof(load);
    if (sysctlbyname("vm.loadavg", &load, &load_size, nullptr, 0) == -1) {
        LOG_ERROR("sysctlbyname returned -1 calling vm.loadavg");
        return nullopt;
    } else {
        LOG_INFO("sysctlbyname returned 1 min: {}, 5 min: {}, 15 min: {}",
                 load.ldavg[0] / static_cast<double>(load.fscale),
                 load.ldavg[1] / static_cast<double>(load.fscale),
                 load.ldavg[2] / static_cast<double>(load.fscale));
        return make_optional(load.ldavg[0] / static_cast<double>(load.fscale));
    }
}

} // namespace

namespace mmotd::platform {

LoadAverageDetails GetLoadAverageDetails() {
    auto cpu_count_holder = GetCpuCount();
    int32_t cpu_count = cpu_count_holder ? *cpu_count_holder : int32_t{0};

    auto load_average_holder = GetSystemLoadAverage();
    double load_average = load_average_holder ? *load_average_holder : double{0.0};

    return make_tuple(cpu_count, load_average);
}

} // namespace mmotd::platform
#endif
