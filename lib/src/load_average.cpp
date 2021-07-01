// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/load_average.h"
#include "lib/include/platform/load_average.h"

using namespace std;

bool gLinkLoadAverage = false;

namespace mmotd::information {

static const bool load_average_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::LoadAverage>(); });

void LoadAverage::FindInformation() {
    auto load_average_holder = mmotd::platform::GetLoadAverageDetails();
    if (!load_average_holder.has_value()) {
        return;
    }
    auto load_average = GetInfoTemplate(InformationId::ID_LOAD_AVERAGE_LOAD_AVERAGE);
    load_average.SetValueArgs(*load_average_holder);
    AddInformation(load_average);
}

} // namespace mmotd::information
