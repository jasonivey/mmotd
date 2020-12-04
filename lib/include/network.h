// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "lib/include/information_provider.h"

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/asio/ip/address.hpp>

namespace mmotd {

struct MacAddress {
    MacAddress() = default;
    MacAddress(const std::uint8_t *buffer, size_t buffer_size);

    explicit operator bool() const { return !IsZero(); }
    std::string to_string() const;
    static MacAddress from_string(const std::string &input_str);

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
};

using NetworkDevices = std::unordered_map<std::string, NetworkDevice>;

class NetworkInfo : public InformationProvider {
public:
    NetworkInfo() = default;

    std::string GetName() const override { return std::string{"network"}; }
    bool QueryInformation() override;
    std::optional<mmotd::ComputerValues> GetInformation() const override;

private:
    bool TryDiscovery();
    std::optional<IpAddress> GetActiveInterface();

    ComputerValues network_information_;
};

} // namespace mmotd
