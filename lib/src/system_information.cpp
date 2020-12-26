// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/platform/system_information.h"
#include "lib/include/system_information.h"

#include <optional>
#include <tuple>
#include <vector>

using namespace std;

bool gLinkSystemInformation = false;

namespace mmotd {

static const bool system_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::SystemInformation>(); });

bool SystemInformation::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        details_ = mmotd::platform::GetSystemInformationDetails();
        return !details_.empty();
    }
    return has_queried;
}

optional<ComputerValues> SystemInformation::GetInformation() const {
    return details_.empty() ? nullopt : make_optional(details_);
}

} // namespace mmotd
