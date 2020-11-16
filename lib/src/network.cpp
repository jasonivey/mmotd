// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
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
#include <fmt/format.h>
#include <plog/Log.h>
#include <boost/numeric/conversion/cast.hpp>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using boost::asio::ip::make_address;
using fmt::format;
using namespace std;

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

string NetworkInfo::to_string() const {
    return ::to_string(network_devices_);
}

string to_string(const NetworkDevices &network_devices) {
    auto str = string{};
    auto i = size_t{0};
    for (const auto &[interface_name, network_device] : network_devices) {
        str += format("{:2}. {}\n{}\n", i++, interface_name, to_string(network_device));
    }
    return str;
}

string NetworkDevice::to_string() const {
    string str;
    const auto spaces = string(fmt::formatted_size("{}", interface_name), ' ');
    if (static_cast<bool>(mac_address)) {
        str += format("{} : {}\n", spaces, mac_address.to_string());
    } else {
        str += format("{} : no mac address\n", spaces);
    }
    if (ip_addresses.empty()) {
        str += format("{} : no ip address\n", spaces);
    } else {
        for (auto i = size_t{0}; i < ip_addresses.size(); ++i) {
            str += format("{} : {} {}\n",
                          spaces,
                          ip_addresses[i].to_string(),
                          ip_addresses[i].is_v4() ? "(ipv4)" : "(ipv6)");
        }
    }
    return str;
}

string to_string(const IpAddresses &ip_addresses) {
    auto str = string{};
    for (const auto &ip_address : ip_addresses) {
        str += ip_address.to_string();
    }
    str.resize(str.size() - 1);
    return str;
}

#if defined(__linux__)

// bool get_mac_address(char* mac_addr, const char* if_name = "eth0")
static optional<NetworkDevices> DiscoverNetworkDevices() {
    //struct ifreq ifinfo;
    //strcpy(ifinfo.ifr_name, if_name);
    //int sd = socket(AF_INET, SOCK_DGRAM, 0);
    //int result = ioctl(sd, SIOCGIFHWADDR, &ifinfo);
    //close(sd);

    //if ((result == 0) && (ifinfo.ifr_hwaddr.sa_family == 1)) {
    //    memcpy(mac_addr, ifinfo.ifr_hwaddr.sa_data, IFHWADDRLEN);
    //    return true;
    //} else {
    //    return false;
    //}
    return nullopt;
}

#elif defined(__APPLE__)

static optional<NetworkDevices> DiscoverNetworkDevices() {
    struct ifaddrs *addrs = nullptr;
    if (getifaddrs(&addrs) != 0) {
        PLOG_ERROR << format("getifaddrs failed, errno: {}", errno);
        return optional<NetworkDevices>{};
    }

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
                PLOG_WARNING
                    << format("{} found mac address {} which is not of the form interface:xx.xx.xx.xx.xx.xx",
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
            if (static_cast<bool>(network_device->mac_address)) {
                PLOG_WARNING << format("{} with mac address {} is being replaced with {}",
                                                     interface_name,
                                                     to_string(network_device->mac_address),
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
                PLOG_INFO << format("{} with ip address {} is unspecified",
                                                  interface_name,
                                                  ip_address.to_string());
            } else if (ip_address.is_loopback()) {
                PLOG_INFO << format("{} with ip address {} is loopback device",
                                                  interface_name,
                                                  ip_address.to_string());
            } else {
                PLOG_INFO << format("{} with ip address {} is multicast device: {}",
                                                  interface_name,
                                                  ip_address.to_string(),
                                                  ip_address.is_multicast() ? "yes" : "no");
                network_device->ip_addresses.push_back(ip_address);
            }
        }
    }
    freeifaddrs(addrs);

    auto cleaned_network_devices = NetworkDevices{};
    copy_if(begin(network_devices),
            end(network_devices),
            inserter(cleaned_network_devices, cleaned_network_devices.begin()),
            [](const auto &device) {
                const auto &network_device = device.second;
                if (network_device.interface_name.empty()) {
                    return false;
                } else if (!static_cast<bool>(network_device.mac_address)) {
                    return false;
                } else if (network_device.ip_addresses.empty()) {
                    return false;
                } else {
                    for (const auto &ip_address : network_device.ip_addresses) {
                        if (ip_address.is_unspecified() || ip_address.is_loopback()) {
                            return false;
                        }
                    }
                }
                return true;
            });
    return make_optional(cleaned_network_devices);
}
#else

#    error no definition for DiscoverNetworkDevices() on this platform!

#endif

bool NetworkInfo::TryDiscovery() {
    const auto devices = DiscoverNetworkDevices();
    if (devices) {
        network_devices_ = *devices;
    }
    return static_cast<bool>(devices);
}

ostream &operator<<(ostream &out, const NetworkInfo &network_info) {
    out << network_info.network_devices_ << "\n";
    return out;
}

ostream &operator<<(ostream &out, const NetworkDevices &network_devices) {
    for (const auto &[interface_name, network_device] : network_devices) {
        out << interface_name << "\n" << network_device << "\n";
    }
    return out;
}

ostream &operator<<(ostream &out, const NetworkDevice &network_device) {
    out << "  interface    : " << network_device.interface_name << "\n";
    out << "  mac address  : " << network_device.mac_address << "\n";
    out << "  ip addresses : " << network_device.ip_addresses << "\n";
    return out;
}

ostream &operator<<(ostream &out, const IpAddresses &ip_addresses) {
    if (ip_addresses.empty()) {
        out << "\n";
    }
    for (auto i = size_t{1}; i < ip_addresses.size(); ++i) {
        if (i == 1) {
            out << ip_addresses[i] << "\n";
        } else if (i + 1 == ip_addresses.size()) {
            out << format("               : {}\n", ip_addresses[i].to_string());
        } else {
            out << format("               : {}\n", ip_addresses[i].to_string());
        }
    }
    return out;
}

ostream &operator<<(ostream &out, const MacAddress &mac_address) {
    out << to_string(mac_address) << "\n";
    return out;
}
