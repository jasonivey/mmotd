// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/platform_error.h"
#include "lib/include/computer_information.h"
#include "lib/include/load_average.h"

#include <ctime>
#include <optional>

#include <fmt/format.h>
#include <plog/Log.h>

#include <sys/sysctl.h>
#include <time.h>

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
    details_.push_back(make_tuple("load average", format("system load average: {:.02f}%", system_load_average)));
    details_.push_back(make_tuple("processor count", to_string(cpu_count)));
    return true;
}

} // namespace mmotd
