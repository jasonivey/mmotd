// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
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
    // UNIT TEST -- assert that all args are not empty!!!
}

bool NetworkDevice::HasActiveBeenTested() const {
    return !boost::indeterminate(active);
}

bool NetworkDevice::IsActive() const {
    return boost::indeterminate(active) ? false : static_cast<bool>(active);
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

bool NetworkDevices::HasActiveBeenTested(const string &interface_name) const {
    auto &network_device = CreateOrFindInterface(interface_name);
    return network_device.HasActiveBeenTested();
}

bool NetworkDevices::IsActive(const string &interface_name) const {
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
            return true;
        } else if (!device.HasActiveBeenTested() || !device.IsActive()) {
            return true;
        } else if (!device.mac_address) {
            return true;
        } else if (device.ip_addresses.empty()) {
            return true;
        } else {
            auto j = find_if(std::begin(device.ip_addresses), std::end(device.ip_addresses), [](const auto &ip) {
                return ip.is_v4();
            });
            return j == std::end(device.ip_addresses);
        }
    });
    devices_.erase(i, std::end(devices_));
}

NetworkDevice &NetworkDevices::CreateOrFindInterface(const string &interface_name) {
    auto i = find_if(std::begin(devices_), std::end(devices_), [&interface_name](auto &device) {
        return interface_name == device.interface_name;
    });
    if (i == std::end(devices_)) {
        devices_.push_back(NetworkDevice{interface_name});
        return devices_.back();
    } else {
        auto &device = *i;
        return device;
    }
}

const NetworkDevice &NetworkDevices::CreateOrFindInterface(const string &interface_name) const {
    auto i = find_if(std::cbegin(devices_), std::cend(devices_), [&interface_name](auto &device) {
        return interface_name == device.interface_name;
    });
    if (i == std::cend(devices_)) {
        // WARNING: this is only to avoid throwing an exception when item is not found.
        //  If the app happens to run on read-only memory this could cause irreparable damage... app will die!
        auto *non_const_devices = const_cast<vector<NetworkDevice> *>(&devices_);
        non_const_devices->push_back(NetworkDevice{interface_name});
        return non_const_devices->back();
    } else {
        auto &device = *i;
        return device;
    }
}

} // namespace mmotd::networking
