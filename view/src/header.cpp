// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/computer_information_provider_factory.h"
#include "view/include/header.h"

using namespace std;

bool gLinkHeaderProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::Header>(); });

optional<string> mmotd::Header::QueryInformation() {
    return nullopt;
}

string mmotd::Header::GetName() const {
    return "header";
}
