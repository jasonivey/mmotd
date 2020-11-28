// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/computer_information_provider_factory.h"
#include "view/include/memory_usage.h"

using namespace std;

bool gLinkMemoryUsageProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::MemoryUsage>(); });

optional<string> mmotd::MemoryUsage::QueryInformation() {
    return nullopt;
}

string mmotd::MemoryUsage::GetName() const {
    return "memory-usage";
}
