// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "lib/include/mac_address.h"
#include "common/include/algorithm.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <unordered_map>
#include <tuple>
#include <vector>

#include <boost/asio/ip/address.hpp>
#include <boost/logic/tribool.hpp>

namespace mmotd::platform {

using IpAddress = boost::asio::ip::address;
using IpAddresses = std::vector<IpAddress>;

struct NetworkDevice {
    std::string interface_name;
    MacAddress mac_address;
    IpAddresses ip_addresses;
    boost::tribool active = boost::indeterminate;
};

class NetworkDevices {
public:
    NetworkDevices() = default;

    void AddInterface(const std::string &interface_name);
    void AddMacAddress(const std::string &interface_name, const mmotd::MacAddress &mac_address);
    void AddIpAddress(const std::string &interface_name, const IpAddress &ip);
    boost::tribool GetActive(const std::string &interface_name) const;
    void SetActive(const std::string &interface_name, bool active);

    void FilterWorthless();

    auto begin() { return std::begin(devices_); }
    auto begin() const { return std::begin(devices_); }
    auto end() { return std::end(devices_); }
    auto end() const { return std::end(devices_); }

private:
    using NetDevices = std::unordered_map<std::string, NetworkDevice>;

    NetworkDevice &CreateOrFindInterface(const std::string &interface_name);
    const NetworkDevice &CreateOrFindInterface(const std::string &interface_name) const;

    mutable NetDevices devices_;
};

using NetworkDetail = std::tuple<std::string, std::string>;
using NetworkDetails = std::vector<NetworkDetail>;

NetworkDetails GetNetworkDetails();

// Inline function definitions
inline void NetworkDevices::AddInterface(const std::string &interface_name) {
    CreateOrFindInterface(interface_name);
}

inline void NetworkDevices::AddMacAddress(const std::string &interface_name, const mmotd::MacAddress &mac_address) {
    auto &network_device = CreateOrFindInterface(interface_name);
    network_device.mac_address = mac_address;
}

inline void NetworkDevices::AddIpAddress(const std::string &interface_name, const IpAddress &ip) {
    auto &network_device = CreateOrFindInterface(interface_name);
    network_device.ip_addresses.push_back(ip);
}

inline boost::tribool NetworkDevices::GetActive(const std::string &interface_name) const {
    auto &network_device = CreateOrFindInterface(interface_name);
    return network_device.active;
}

inline void NetworkDevices::SetActive(const std::string &interface_name, bool active) {
    auto &network_device = CreateOrFindInterface(interface_name);
    network_device.active = active;
}

inline void NetworkDevices::FilterWorthless() {
    mmotd::algorithms::erase_if(devices_, [](const auto& item) {
        auto const &[name, device] = item;
        if (name.empty() || device.interface_name.empty()) {
            return true;
        } else if (!boost::indeterminate(device.active) && !static_cast<bool>(device.active)) {
            return true;
        } else if (!device.mac_address) {
            return true;
        } else if (device.ip_addresses.empty()) {
            return true;
        } else {
            auto j = std::find_if(std::begin(device.ip_addresses),
                                  std::end(device.ip_addresses),
                                  [](const auto &ip) { return ip.is_v4(); });
            return j == std::end(device.ip_addresses);
        }
        return false;
    });
}

inline NetworkDevice &NetworkDevices::CreateOrFindInterface(const std::string &interface_name) {
    auto i = devices_.find(interface_name);
    if (i == std::end(devices_)) {
        i = devices_.insert(std::begin(devices_), {interface_name, NetworkDevice{}});
        auto &network_device = (*i).second;
        network_device.interface_name = interface_name;
    }
    return (*i).second;
}

inline const NetworkDevice &NetworkDevices::CreateOrFindInterface(const std::string &interface_name) const {
    // DO NOT DO!
    return const_cast<NetworkDevices *>(this)->CreateOrFindInterface(interface_name);
}

} // namespace mmotd
