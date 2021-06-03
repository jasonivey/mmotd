// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(_WIN32)
#include "lib/include/platform/system_information.h"

#include <string>
#include <tuple>
#include <vector>

using namespace std;

namespace mmotd::platform {

SystemInformationDetails GetSystemInformationDetails() {
    return SystemInformationDetails{};
}

} // namespace mmotd::platform
#endif
