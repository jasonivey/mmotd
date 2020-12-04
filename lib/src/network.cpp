// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/platform_error.h"
#include "lib/include/computer_information.h"
#include "lib/include/network.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <fmt/format.h>
#include <iostream>
#include <optional>
#include <plog/Log.h>
#include <scope_guard.hpp>
#include <stdexcept>
#include <string>
#include <vector>

using boost::asio::ip::make_address;
using fmt::format;
using namespace std;

bool gLinkNetworkInformation = false;

namespace mmotd {

static const bool network_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::NetworkInfo>(); });

static optional<NetworkDevices> DiscoverNetworkDevices(const IpAddress &ip_address);

MacAddress::MacAddress(const uint8_t *buffer, size_t buffer_size) {
    if (buffer_size != MAC_ADDRESS_SIZE) {
        throw std::runtime_error("invalid size of mac address");
    } else if (buffer != nullptr) {
        memcpy(&data_[0], buffer, MAC_ADDRESS_SIZE);
    } else {
        memset(&data_[0], 0, MAC_ADDRESS_SIZE);
    }
}

MacAddress::MacAddress(const string &input_str) {
    using namespace boost;
    using split_vector_type = vector<string>;
    split_vector_type mac_addr_parts;
    split(mac_addr_parts, input_str, is_any_of(" :."), token_compress_on);
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
    vector<uint8_t> mac_addr;
    for (const auto &part : mac_addr_parts) {
        if (all(part, is_xdigit()) && part.size() <= 2) {
            char *end = nullptr;
            auto value = strtoul(&part[0], &end, 16);
            mac_addr.push_back(numeric_cast<uint8_t>(value));
        } else {
            throw std::runtime_error("invalid character found in mac address string");
        }
    }
    memcpy(&data_[0], &mac_addr[0], MAC_ADDRESS_SIZE);
}

bool MacAddress::IsZero() const {
    static constexpr array<uint8_t, MAC_ADDRESS_SIZE> EMPTY_MAC_ADDRESS = {0, 0, 0, 0, 0, 0};
    return memcmp(data_.data(), &EMPTY_MAC_ADDRESS, MAC_ADDRESS_SIZE) == 0;
}

string MacAddress::to_string() const {
    return format("{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
                  static_cast<uint32_t>(data_[0]),
                  static_cast<uint32_t>(data_[1]),
                  static_cast<uint32_t>(data_[2]),
                  static_cast<uint32_t>(data_[3]),
                  static_cast<uint32_t>(data_[4]),
                  static_cast<uint32_t>(data_[5]));
}

bool NetworkInfo::TryDiscovery() {
    auto active_ip_address_wrapper = GetActiveInterface();
    if (!active_ip_address_wrapper) {
        PLOG_INFO << "unable to find active ip address";
    }
    auto active_ip_address = active_ip_address_wrapper.value_or(IpAddress{});

    auto devices = DiscoverNetworkDevices(active_ip_address);
    if (!devices) {
        PLOG_INFO << "no network devices were discovered";
        return false;
    }

    auto network_information = ComputerValues{};
    for (auto [key, value] : *devices) {
        assert(key == value.interface_name);
        network_information.push_back(
            // ☞ WHITE RIGHT POINTING INDEX, Unicode: U+261E, UTF-8: E2 98 9E
            make_tuple("network info", format("{}☞{}", key, value.mac_address.to_string())));
        for (auto ip_address : value.ip_addresses) {
            network_information.push_back(make_tuple("network info", format("{}☞{}", key, ip_address.to_string())));
        }
    }
    network_information_ = network_information;
    return true;
}

bool NetworkInfo::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        return TryDiscovery();
    }
    return has_queried;
}

std::optional<mmotd::ComputerValues> NetworkInfo::GetInformation() const {
    return network_information_.empty() ? nullopt : make_optional(network_information_);
}

#if defined(__linux__)

static optional<NetworkDevices> DiscoverNetworkDevices(const IpAddress &ip_address) {
    return nullopt;
}

#elif defined(__APPLE__)

static optional<NetworkDevices> DiscoverNetworkDevices(const IpAddress &active_ip_address) {
    struct ifaddrs *addrs = nullptr;
    if (getifaddrs(&addrs) != 0) {
        PLOG_ERROR << format("getifaddrs failed, errno: {}", errno);
        return nullopt;
    }
    auto freeifaddrs_deleter = sg::make_scope_guard([addrs]() { freeifaddrs(addrs); });

    auto network_devices = NetworkDevices{};
    static constexpr size_t BUFFER_SIZE = max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN) + 1;
    array<char, BUFFER_SIZE> buffer = {0};
    for (struct ifaddrs *ptr = addrs; ptr; ptr = ptr->ifa_next) {
        if (ptr->ifa_addr == nullptr || ptr->ifa_name == nullptr) {
            continue;
        }
        const auto interface_name = string{ptr->ifa_name};
        const auto address_family = ptr->ifa_addr->sa_family;
        if (address_family == AF_LINK) {
            const auto *sock_addr = reinterpret_cast<const struct sockaddr_dl *>(ptr->ifa_addr);
            const auto ntoa_str = string(link_ntoa(sock_addr));
            const auto index = ntoa_str.find(':');
            if (index == string::npos) {
                PLOG_WARNING << format("{} found mac address {} which is not of the form interface:xx.xx.xx.xx.xx.xx",
                                       interface_name,
                                       ntoa_str);
                continue;
            }
            auto iter = network_devices.find(interface_name);
            NetworkDevice *network_device = iter == end(network_devices) ? nullptr : &(iter->second);
            if (iter == end(network_devices)) {
                iter = network_devices.insert(cbegin(network_devices), {interface_name, NetworkDevice{}});
                network_device = &(iter->second);
                network_device->interface_name = interface_name;
            }
            auto mac_address = ntoa_str.substr(index + 1);
            if (network_device->mac_address) {
                PLOG_WARNING << format("{} with mac address {} is being replaced with {}",
                                       interface_name,
                                       network_device->mac_address.to_string(),
                                       mac_address);
            }
            network_device->mac_address = MacAddress(mac_address);
        } else if (address_family == AF_INET || address_family == AF_INET6) {
            auto ip_str = string{};
            buffer.fill(0);
            if (address_family == AF_INET) {
                const auto *sock_addr = reinterpret_cast<struct sockaddr_in *>(ptr->ifa_addr);
                if (inet_ntop(AF_INET, &sock_addr->sin_addr, buffer.data(), INET_ADDRSTRLEN)) {
                    ip_str.assign(buffer.data());
                }
            } else {
                const auto *sock_addr = reinterpret_cast<struct sockaddr_in6 *>(ptr->ifa_addr);
                if (inet_ntop(AF_INET6, &sock_addr->sin6_addr, buffer.data(), INET6_ADDRSTRLEN)) {
                    ip_str.assign(buffer.data());
                }
            }
            const auto family_name = address_family == AF_INET ? string{"ipv4"} : string{"ipv6"};
            PLOG_INFO << format("{} found ip {} for family {}", interface_name, ip_str, family_name);
            auto iter = network_devices.find(interface_name);
            NetworkDevice *network_device = iter == end(network_devices) ? nullptr : &(iter->second);
            if (iter == end(network_devices)) {
                iter = network_devices.insert(cbegin(network_devices), {interface_name, NetworkDevice{}});
                network_device = &(iter->second);
                network_device->interface_name = interface_name;
            }
            auto ip_address = make_address(ip_str);
            if (ip_address.is_unspecified()) {
                PLOG_INFO << format("{} with ip address {} is unspecified", interface_name, ip_address.to_string());
            } else if (ip_address.is_loopback()) {
                PLOG_INFO << format("{} with ip address {} is loopback device", interface_name, ip_address.to_string());
            } else {
                PLOG_INFO << format("{} with ip address {} is multicast device: {}",
                                    interface_name,
                                    ip_address.to_string(),
                                    ip_address.is_multicast() ? "yes" : "no");
                network_device->ip_addresses.push_back(ip_address);
            }
        }
    }

    auto cleaned_network_devices = NetworkDevices{};
    copy_if(begin(network_devices),
            end(network_devices),
            inserter(cleaned_network_devices, cleaned_network_devices.begin()),
            [active_ip_address](const auto &device) {
                const auto &network_device = device.second;
                if (network_device.interface_name.empty()) {
                    return false;
                } else if (!network_device.mac_address) {
                    return false;
                } else if (network_device.ip_addresses.empty()) {
                    return false;
                } else {
                    if (!active_ip_address.is_unspecified()) {
                        auto i = find(begin(network_device.ip_addresses),
                                      end(network_device.ip_addresses),
                                      active_ip_address);
                        if (i == end(network_device.ip_addresses)) {
                            return false;
                        }
                    }
                    auto i = find_if(
                        begin(network_device.ip_addresses),
                        end(network_device.ip_addresses),
                        [](const auto &ip_address) { return ip_address.is_unspecified() || ip_address.is_loopback(); });
                    if (i != end(network_device.ip_addresses)) {
                        return false;
                    }
                }
                return true;
            });
    return make_optional(cleaned_network_devices);
}
#else

#    error no definition for DiscoverNetworkDevices(const IpAddress &ip_address) on this platform!

#endif

optional<IpAddress> NetworkInfo::GetActiveInterface() {
    auto sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        PLOG_ERROR << "unable to open AF_INET SOCK_DGRAM socket";
        return nullopt;
    }
    auto socket_closer = sg::make_scope_guard([sock]() { close(sock); });

    constexpr const char *GOOGLE_DNS_IP = "8.8.8.8";
    constexpr const uint16_t DNS_PORT = 53;
    auto serv = sockaddr_in{};
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(GOOGLE_DNS_IP);
    serv.sin_port = htons(DNS_PORT);

    auto retval = connect(sock, reinterpret_cast<const sockaddr *>(&serv), sizeof(serv));
    if (retval == -1) {
        PLOG_ERROR << format("connect failed to {} on port {}", GOOGLE_DNS_IP, DNS_PORT);
        return nullopt;
    }

    auto name = sockaddr_in{};
    socklen_t namelen = sizeof(name);
    retval = getsockname(sock, reinterpret_cast<sockaddr *>(&name), &namelen);
    if (retval == -1) {
        PLOG_ERROR << format("getsockname failed after connecting to {} on port {}", GOOGLE_DNS_IP, DNS_PORT);
        return nullopt;
    }

    auto buffer = vector<char>(64);
    const char *address_str = inet_ntop(AF_INET, &name.sin_addr, buffer.data(), buffer.size());
    if (address_str == nullptr) {
        PLOG_ERROR << format("inet_ntop failed after connecting to {} on port {}, details: {}",
                             GOOGLE_DNS_IP,
                             DNS_PORT,
                             mmotd::platform::error::to_string(errno));
        return nullopt;
    }
    PLOG_INFO << format("found active interface {} by connecting to {} on port {}",
                        address_str,
                        GOOGLE_DNS_IP,
                        DNS_PORT);
    auto ip_address = make_address(address_str);
    return make_optional(ip_address);
}

} // namespace mmotd
