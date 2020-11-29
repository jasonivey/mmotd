// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/computer_information_provider_factory.h"
#include "view/include/disk_usage.h"

using namespace std;

bool gLinkDiskUsageProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::DiskUsage>(); });

optional<string> mmotd::DiskUsage::QueryInformation() {
    return nullopt;
}

string mmotd::DiskUsage::GetName() const {
    return "disk usage";
}
