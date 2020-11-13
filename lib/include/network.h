// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <array>
#include <boost/asio/ip/address.hpp>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <unordered_map>
#include <vector>

struct MacAddress {
    friend std::ostream &operator<<(std::ostream &out, const MacAddress &mac_address);

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

inline std::string to_string(const MacAddress &mac_address) {
    return mac_address.to_string();
}

using IpAddress = boost::asio::ip::address;
using IpAddresses = std::vector<IpAddress>;

std::string to_string(const IpAddresses &ip_addresses);

struct NetworkDevice {
    std::string interface_name;
    MacAddress mac_address;
    IpAddresses ip_addresses;

    std::string to_string() const;
};

inline std::string to_string(const NetworkDevice &network_device) {
    return network_device.to_string();
}

using NetworkDevices = std::unordered_map<std::string, NetworkDevice>;

class NetworkInfo {
    friend std::ostream &operator<<(std::ostream &out, const NetworkInfo &network_info);

public:
    NetworkInfo() = default;
    bool TryDiscovery();

    std::string to_string() const;

private:
    NetworkDevices network_devices_;
};

std::string to_string(const NetworkDevices &network_devices);
inline std::string to_string(const NetworkInfo &network_info) {
    return network_info.to_string();
}

std::ostream &operator<<(std::ostream &out, const NetworkDevices &network_devices);
std::ostream &operator<<(std::ostream &out, const NetworkDevice &network_device);
std::ostream &operator<<(std::ostream &out, const IpAddresses &ip_addresses);
