// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/computer_information_provider_factory.h"
#include "view/include/processor_count.h"

using namespace std;

bool gLinkProcessorCountProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::ProcessorCount>(); });

optional<string> mmotd::ProcessorCount::QueryInformation() {
    return nullopt;
}

string mmotd::ProcessorCount::GetName() const {
    return "processor-count";
}
