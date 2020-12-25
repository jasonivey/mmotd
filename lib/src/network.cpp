// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/network.h"
#include "lib/include/platform/network.h"

#include <optional>
#include <string>
#include <vector>

using namespace std;

bool gLinkNetworkInformation = false;

namespace mmotd {

static const bool network_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::NetworkInfo>(); });

bool NetworkInfo::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        network_information_ = mmotd::platform::GetNetworkDetails();
        return !network_information_.empty();
    }
    return has_queried;
}

std::optional<mmotd::ComputerValues> NetworkInfo::GetInformation() const {
    return network_information_.empty() ? nullopt : make_optional(network_information_);
}

} // namespace mmotd
