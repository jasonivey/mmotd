// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(_WIN32)
#include "common/include/logging.h"
#include "lib/include/platform/package_management.h"

#include <string>

using namespace std;

namespace mmotd::platform::package_management {

string GetUpdateDetails() {
    LOG_VERBOSE("[windows] getting package management update details");
    return string{};
}

string GetRebootRequired() {
    LOG_VERBOSE("[windows] getting package management reboot required");
    return string{};
}

} // namespace mmotd::platform::package_management
#endif
