// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/platform/swap.h"
#include "lib/include/swap.h"

using namespace std;

bool gLinkSwapUsage = false;

namespace mmotd {

static const bool swap_usage_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::Swap>(); });

bool Swap::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        details_ = mmotd::platform::GetSwapDetails();
        return !details_.empty();
    }
    return has_queried;
}

optional<mmotd::ComputerValues> Swap::GetInformation() const {
    return !details_.empty() ? make_optional(details_) : nullopt;
}

} // namespace mmotd
