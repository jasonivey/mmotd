#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/numeric/conversion/cast.hpp>

struct MacAddress {
    MacAddress() = default;
    MacAddress(const std::uint8_t *buffer, size_t buffer_size) {
        Initialize(buffer, buffer_size);
    }
    MacAddress(const std::string &input_str) {
        using namespace std;
        typedef vector<string> split_vector_type;
        split_vector_type mac_addr_parts;
        boost::split(mac_addr_parts, input_str, boost::is_any_of(" :."), boost::token_compress_on);
        if (mac_addr_parts.size() == 1) {
            // it didn't split at all -- it better be 2 characters for a total of 12!
            mac_addr_parts.clear();
            auto part = string{};
            for (auto ch : input_str) {
                part.push_back(ch);
                if (part.size() == 2) {
                    mac_addr_parts.push_back(part);
                    part.clear();
                }
            }
        }
        if (mac_addr_parts.size() != MAC_ADDRESS_SIZE) {
            throw std::runtime_error("mac address is not the correct length");
        }
        std::vector<std::uint8_t> mac_addr;
        for (const auto &part : mac_addr_parts) {
            if (boost::all(part, boost::is_xdigit()) && part.size() <= 2) {
                char *end = nullptr;
                auto value = strtoul(&part[0], &end, 16);
                mac_addr.push_back(boost::numeric_cast<uint8_t>(value));
            } else {
                throw std::runtime_error("invalid character found in mac address string");
            }
        }
        memcpy(&data_[0], &mac_addr[0], MAC_ADDRESS_SIZE);
    }

private:
    void Initialize(const std::uint8_t *buffer, size_t buffer_size) {
        if (buffer_size != MAC_ADDRESS_SIZE) {
            throw std::runtime_error("invalid size of mac address");
        } else if (buffer != nullptr) {
            memcpy(&data_[0], buffer, MAC_ADDRESS_SIZE);
        } else {
            memset(&data_[0], 0, MAC_ADDRESS_SIZE);
        }
    }

    static const size_t MAC_ADDRESS_SIZE = 6;
    std::array<std::uint8_t, MAC_ADDRESS_SIZE> data_;
};

struct NetworkDevice {
    std::string interface_name;
    MacAddress mac_address;
    boost::asio::ip::address ip_address;
};

typedef std::vector<NetworkDevice> NetworkDevices;

class NetworkInfo {
public:
    NetworkInfo() = default;

    bool QueryForNetworkInfo();

private:
    NetworkDevices network_devices_;
};
