// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/active_network_interfaces.h"
#include "view/include/computer_information_provider_factory.h"

using namespace std;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::ActiveNetworkInterfaces>(); });

optional<string> mmotd::ActiveNetworkInterfaces::QueryInformation() {
    return nullopt;
}

string mmotd::ActiveNetworkInterfaces::GetName() const {
    return "active-network-interfaces";
}
