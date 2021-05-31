// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/network.h"
#include "lib/include/platform/network.h"

#include <optional>
#include <string>
#include <vector>

#include <fmt/format.h>

using fmt::format;
using namespace std;

bool gLinkNetworkInformation = false;

namespace mmotd::information {

static const bool network_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::NetworkInfo>(); });

void NetworkInfo::FindInformation() {
    auto network_devices = mmotd::platform::GetNetworkDevices();
    for (const auto &network_device : network_devices) {
        auto interface_name_info = GetInfoTemplate(InformationId::ID_NETWORK_INFO_INTERFACE_NAME);
        interface_name_info.SetValueArgs(network_device.interface_name);
        AddInformation(interface_name_info);

        auto mac_info = GetInfoTemplate(InformationId::ID_NETWORK_INFO_MAC);
        mac_info.SetValueArgs(network_device.mac_address.to_string());
        AddInformation(mac_info);

        for (const auto &ip : network_device.ip_addresses) {
            auto ip_info = GetInfoTemplate(InformationId::ID_NETWORK_INFO_IP);
            ip_info.SetValueArgs(ip.to_string());
            AddInformation(ip_info);
        }
    }
}

} // namespace mmotd::information
