// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/computer_information_provider_factory.h"
#include "view/include/sub_header.h"

using namespace std;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::SubHeader>(); });

optional<string> mmotd::SubHeader::QueryInformation() {
    return nullopt;
}

string mmotd::SubHeader::GetName() const {
    return "sub-header";
}
