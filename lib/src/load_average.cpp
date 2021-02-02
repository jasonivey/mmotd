// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/load_average.h"
#include "lib/include/platform/load_average.h"

using namespace std;

bool gLinkLoadAverage = false;

namespace mmotd::information {

static const bool load_average_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::LoadAverage>(); });

bool LoadAverage::FindInformation() {
    if (auto details = mmotd::platform::GetLoadAverageDetails(); !details.empty()) {
        for (auto [name, value] : details) {
            if (name == "processor count") {
                auto processor_count = GetInfoTemplate(InformationId::ID_PROCESSOR_COUNT_PROCESSOR_CORE_COUNT);
                processor_count.information = value;
                AddInformation(processor_count);
            } else if (name == "load average") {
                auto load_average = GetInfoTemplate(InformationId::ID_LOAD_AVERAGE_LOAD_AVERAGE);
                load_average.information = value;
                AddInformation(load_average);
            }
        }
        return true;
    } else {
        return false;
    }
}

} // namespace mmotd::information
