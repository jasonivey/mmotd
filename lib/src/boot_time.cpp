// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/chrono_io.h"
#include "common/include/posix_error.h"
#include "lib/include/boot_time.h"
#include "lib/include/computer_information.h"
#include "lib/include/platform/boot_time.h"

#include <chrono>
#include <ctime>
#include <iterator>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkBootTime = false;

namespace mmotd {

static const bool boot_time_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::BootTime>(); });

bool BootTime::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        return GetBootTime();
    }
    return has_queried;
}

optional<mmotd::ComputerValues> BootTime::GetInformation() const {
    return !details_.empty() ? make_optional(details_) : nullopt;
}

bool BootTime::GetBootTime() {
    details_.clear();
    auto random_boot_time_wrapper = mmotd::platform::GetBootTime();
    if (!random_boot_time_wrapper) {
        return false;
    }

    auto boot_time = *random_boot_time_wrapper;
    details_.push_back(make_tuple("boot time", boot_time));
    return !details_.empty();
}

} // namespace mmotd
