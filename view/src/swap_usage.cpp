// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/swap_usage.h"
#include "view/include/computer_information_provider_factory.h"

using namespace std;

static const bool factory_registered = mmotd::RegisterComputerInformationProvider([](){
    return make_unique<mmotd::SwapUsage>();
});

optional<string> mmotd::SwapUsage::QueryInformation() {
    return nullopt;
}

string mmotd::SwapUsage::GetName() const {
    return "swap-usage";
}
