// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/computer_information_provider_factory.h"
#include "view/include/computer_name.h"

using namespace std;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::ComputerName>(); });

optional<string> mmotd::ComputerName::QueryInformation() {
    return nullopt;
}

string mmotd::ComputerName::GetName() const {
    return "computer-name";
}
