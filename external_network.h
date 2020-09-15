// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <boost/asio/ip/address.hpp>
#include <cstdint>
#include <string>

using IpAddress = boost::asio::ip::address;

class ExternalNetwork {
public:
    ExternalNetwork() = default;

    bool TryDiscovery();
    std::string to_string() const;

private:
    IpAddress ip_address_;
};

inline std::string to_string(const ExternalNetwork &external_network) { return external_network.to_string(); }
