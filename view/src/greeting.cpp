// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/computer_information_provider_factory.h"
#include "view/include/greeting.h"

using namespace std;

bool gLinkGreetingProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::Greeting>(); });

optional<string> mmotd::Greeting::QueryInformation() {
    return nullopt;
}

string mmotd::Greeting::GetName() const {
    return "greeting";
}
