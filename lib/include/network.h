// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "lib/include/information_provider.h"

#include <array>
#include <boost/asio/ip/address.hpp>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace mmotd {

struct MacAddress {
    // friend std::ostream &operator<<(std::ostream &out, const MacAddress &mac_address);

    MacAddress() = default;
    MacAddress(const std::uint8_t *buffer, size_t buffer_size);
    MacAddress(const std::string &input_str);

    explicit operator bool() const { return !IsZero(); }
    std::string to_string() const;

private:
    bool IsZero() const;

    static const size_t MAC_ADDRESS_SIZE = 6;
    std::array<std::uint8_t, MAC_ADDRESS_SIZE> data_;
};

using IpAddress = boost::asio::ip::address;
using IpAddresses = std::vector<IpAddress>;

struct NetworkDevice {
    std::string interface_name;
    MacAddress mac_address;
    IpAddresses ip_addresses;

    // std::string to_string() const;
};

using NetworkDevices = std::unordered_map<std::string, NetworkDevice>;

class NetworkInfo : public InformationProvider {
    //friend std::ostream &operator<<(std::ostream &out, const NetworkInfo &network_info);

public:
    NetworkInfo() = default;
    // std::string to_string() const;

    std::string GetName() const override { return std::string{"network"}; }
    bool QueryInformation() override;
    std::optional<mmotd::ComputerValues> GetInformation() const override;

private:
    bool TryDiscovery();

    // NetworkDevices network_devices_;
    ComputerValues network_information_;
};

} // namespace mmotd

// inline std::string to_string(const mmotd::MacAddress &mac_address) {
//     return mac_address.to_string();
// }

// std::string to_string(const mmotd::IpAddresses &ip_addresses);

// inline std::string to_string(const mmotd::NetworkDevice &network_device) {
//     return network_device.to_string();
// }

// std::string to_string(const mmotd::NetworkDevices &network_devices);
// inline std::string to_string(const mmotd::NetworkInfo &network_info) {
//     return network_info.to_string();
// }

// std::ostream &operator<<(std::ostream &out, const mmotd::NetworkDevices &network_devices);
// std::ostream &operator<<(std::ostream &out, const mmotd::NetworkDevice &network_device);
// std::ostream &operator<<(std::ostream &out, const mmotd::IpAddresses &ip_addresses);
