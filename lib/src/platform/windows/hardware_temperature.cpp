// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(_WIN32)
#include "lib/include/platform/hardware_temperature.h"

using namespace std;

namespace {

} // namespace

namespace mmotd::platform {

Temperature GetCpuTemperature() {
    return Temperature{};
}

Temperature GetGpuTemperature() {
    return Temperature{};
}

} // namespace mmotd::platform
#endif
