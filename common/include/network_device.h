// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"
#include "common/include/mac_address.h"

#include <array>
#include <cstdint>
#include <iterator>
#include <string>
#include <vector>

#include <boost/asio/ip/address.hpp>
#include <boost/logic/tribool.hpp>

namespace mmotd::networking {

using IpAddress = boost::asio::ip::address;
using IpAddresses = std::vector<IpAddress>;

struct NetworkDevice {
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(NetworkDevice);

    explicit NetworkDevice(const std::string &interface);

    std::string interface_name;
    IpAddresses ip_addresses;
    MacAddress mac_address;
    boost::tribool active = boost::indeterminate;

    bool HasActiveBeenTested() const;
    bool IsActive() const;
    void SetActive(bool value);
};

class NetworkDevices {
public:
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(NetworkDevices);

    void AddInterface(const std::string &interface_name);
    void AddMacAddress(const std::string &interface_name, const MacAddress &mac_address);
    void AddIpAddress(const std::string &interface_name, const IpAddress &ip);
    bool HasActiveBeenTested(const std::string &interface_name);
    bool IsActive(const std::string &interface_name);
    void SetActive(const std::string &interface_name, bool value);

    void FilterWorthless();

    auto begin() { return std::begin(devices_); }
    auto begin() const { return std::cbegin(devices_); }
    auto end() { return std::end(devices_); }
    auto end() const { return std::cend(devices_); }

private:
    NetworkDevice &CreateOrFindInterface(const std::string &interface_name);

    mutable std::vector<NetworkDevice> devices_;
};

} // namespace mmotd::networking
