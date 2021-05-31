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
    auto details = mmotd::platform::GetLoadAverageDetails();
    auto [processor_cnt, ld_average] = details;

    auto processor_count = GetInfoTemplate(InformationId::ID_PROCESSOR_COUNT_PROCESSOR_CORE_COUNT);
    processor_count.SetValueArgs(processor_cnt);
    AddInformation(processor_count);

    auto load_average = GetInfoTemplate(InformationId::ID_LOAD_AVERAGE_LOAD_AVERAGE);
    load_average.SetValueArgs(ld_average);
    AddInformation(load_average);
}

} // namespace mmotd::information
