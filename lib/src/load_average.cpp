// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/load_average.h"
#include "lib/include/platform/load_average.h"

using namespace std;

bool gLinkLoadAverage = false;

namespace mmotd {

static const bool load_average_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::LoadAverage>(); });

bool LoadAverage::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        details_ = mmotd::platform::GetLoadAverageDetails();
        return !details_.empty();
    }
    return has_queried;
}

optional<mmotd::ComputerValues> LoadAverage::GetInformation() const {
    return !details_.empty() ? make_optional(details_) : nullopt;
}

} // namespace mmotd
