// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/platform/swap.h"
#include "lib/include/swap.h"

using namespace std;

bool gLinkSwapUsage = false;

namespace mmotd::information {

static const bool swap_usage_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::Swap>(); });

bool Swap::FindInformation() {
    if (auto details = mmotd::platform::GetSwapDetails(); !details.empty()) {
        for (const auto &[name, value] : details) {
            if (name == "total") {
                auto obj = GetInfoTemplate(InformationId::ID_SWAP_USAGE_TOTAL);
                obj.information = value;
                AddInformation(obj);
            } else if (name == "percent") {
                auto obj = GetInfoTemplate(InformationId::ID_SWAP_USAGE_PERCENT_USED);
                obj.information = value;
                AddInformation(obj);
            } else if (name == "free") {
                auto obj = GetInfoTemplate(InformationId::ID_SWAP_USAGE_FREE);
                obj.information = value;
                AddInformation(obj);
            }
        }
        return true;
    }
    return false;
}

} // namespace mmotd::information
