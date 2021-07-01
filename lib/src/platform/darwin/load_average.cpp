// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__APPLE__)
#include "common/include/logging.h"
#include "lib/include/platform/load_average.h"

#include <cmath>
#include <ctime>
#include <optional>

#include <fmt/format.h>

#include <sys/sysctl.h>
#include <unistd.h>

using fmt::format;
using namespace std;

namespace {

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

optional<double> GetLoadAverageDetails() {
    auto load_average = GetSystemLoadAverage();
    if (load_average.has_value() && !std::isnan(load_average.value())) {
        return load_average;
    }
    return nullopt;
}

} // namespace mmotd::platform
#endif
