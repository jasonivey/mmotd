// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/algorithm.h"
#include "common/include/network_device.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

using namespace std;

namespace mmotd::networking {

NetworkDevice::NetworkDevice(const string &interface) : interface_name(interface) {
    PRECONDITIONS(!empty(interface), "interface name should never be empty");
}

bool NetworkDevice::HasActiveBeenTested() const {
    auto active_holds_initial_value = boost::indeterminate(active);
    return !active_holds_initial_value;
}

bool NetworkDevice::IsActive() const {
    if (active) {
        return true;
    } else if (!active) {
        return false;
    } else {
        // since active hasn't been set to true/false yet then return 'false'
        CHECKS(boost::indeterminate(active), "active must be indeterminate as we tested for true/false");
        return false;
    }
}

void NetworkDevice::SetActive(bool value) {
    active = value;
}

void NetworkDevices::AddInterface(const string &interface_name) {
    CreateOrFindInterface(interface_name);
}

void NetworkDevices::AddMacAddress(const string &interface_name, const MacAddress &mac_address) {
    auto &network_device = CreateOrFindInterface(interface_name);
    network_device.mac_address = mac_address;
}

void NetworkDevices::AddIpAddress(const string &interface_name, const IpAddress &ip) {
    auto &network_device = CreateOrFindInterface(interface_name);
    network_device.ip_addresses.push_back(ip);
}

bool NetworkDevices::HasActiveBeenTested(const string &interface_name) {
    auto &network_device = CreateOrFindInterface(interface_name);
    return network_device.HasActiveBeenTested();
}

bool NetworkDevices::IsActive(const string &interface_name) {
    auto &network_device = CreateOrFindInterface(interface_name);
    return network_device.IsActive();
}

void NetworkDevices::SetActive(const string &interface_name, bool value) {
    auto &network_device = CreateOrFindInterface(interface_name);
    network_device.SetActive(value);
}

void NetworkDevices::FilterWorthless() {
    auto i = remove_if(std::begin(devices_), std::end(devices_), [](const auto &device) {
        if (device.interface_name.empty()) {
            // if there wasn't a name given to the interface (en1, en5, etc) -- remove device
            return true;
        } else if (!device.HasActiveBeenTested() || !device.IsActive()) {
            // if the device isn't active or couldn't be tested for whether it was active -- remove device
            return true;
        } else if (!device.mac_address) {
            // if the device does not have a mac address -- remove device
            return true;
        } else if (device.ip_addresses.empty()) {
            // if there are no IP addresses at all -- remove device
            return true;
        } else {
            // if there are no IPv4 addresses -- remove device
            auto j = find_if(std::begin(device.ip_addresses), std::end(device.ip_addresses), [](const auto &ip) {
                return ip.is_v4();
            });
            return j == std::end(device.ip_addresses);
        }
    });
    // delete all those devices which were moved to (i, end(devices_)]
    devices_.erase(i, std::end(devices_));
}

NetworkDevice &NetworkDevices::CreateOrFindInterface(const string &interface_name) {
    auto i = find_if(std::begin(devices_), std::end(devices_), [&interface_name](auto &device) {
        return interface_name == device.interface_name;
    });
    if (i == std::end(devices_)) {
        devices_.emplace_back(interface_name);
        return devices_.back();
    } else {
        auto &device = *i;
        return device;
    }
}

} // namespace mmotd::networking
