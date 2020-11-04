// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/host_name.h"
#include "view/include/computer_information_provider_factory.h"

using namespace std;

static const bool factory_registered = mmotd::RegisterComputerInformationProvider([](){
    return make_unique<mmotd::HostName>();
});

optional<string> mmotd::HostName::QueryInformation() {
    return nullopt;
}

string mmotd::HostName::GetName() const {
    return "host-name";
}