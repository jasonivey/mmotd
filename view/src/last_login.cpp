// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/last_login.h"
#include "view/include/computer_information_provider_factory.h"

using namespace std;

static const bool factory_registered = mmotd::RegisterComputerInformationProvider([](){
    return make_unique<mmotd::LastLogin>();
});

optional<string> mmotd::LastLogin::QueryInformation() {
    return nullopt;
}

string mmotd::LastLogin::GetName() const {
    return "last-login";
}
