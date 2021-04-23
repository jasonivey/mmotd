// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/platform/package_management.h"

#include <string>

#include <plog/Log.h>

using namespace std;

namespace mmotd::platform::package_management {

string GetUpdateDetails() {
    PLOG_VERBOSE << "[darwin] getting package management update details";
    return string{};
}

string GetRebootRequired() {
    PLOG_VERBOSE << "[darwin] getting package management reboot required";
    return string{};
}

} // namespace mmotd::platform::package_management
