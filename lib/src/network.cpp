// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/posix_error.h"
#include "lib/include/computer_information.h"
#include "lib/include/network.h"

#include <algorithm>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <fmt/format.h>
#include <plog/Log.h>
#include <scope_guard.hpp>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if_dl.h>
#include <net/if_media.h>
#include <netinet/in.h>
#include <netinet6/in6_var.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using boost::numeric_cast;
using boost::asio::ip::make_address;
using fmt::format;
using namespace std;

bool gLinkNetworkInformation = false;

namespace mmotd {

static const bool network_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::NetworkInfo>(); });

static optional<NetworkDevices> DiscoverNetworkDevices();

MacAddress::MacAddress(const uint8_t *buffer, size_t buffer_size) {
    if (buffer_size != MAC_ADDRESS_SIZE) {
        throw std::invalid_argument("invalid size of mac address");
    } else if (buffer != nullptr) {
        memcpy(&data_[0], buffer, MAC_ADDRESS_SIZE);
    } else {
        memset(&data_[0], 0, MAC_ADDRESS_SIZE);
    }
}

// converts the following strings to a MacAddress:
// '64:4b:f0:27:6a:76', '64.4b.f0.27.6a.76', '64 4b f0 27 6a 76' or '644bf0276a76'
MacAddress MacAddress::from_string(const std::string &input_str) {
    using namespace boost::algorithm;

    auto mac_addr_hex_chars = vector<string>{};
    split(mac_addr_hex_chars, input_str, is_any_of(" :."), token_compress_on);

    // if it didn't split at all -- the 6 double digit hex chars must not be delimitted by anything
    if (mac_addr_hex_chars.size() == 1) {
        mac_addr_hex_chars.clear();
        auto hex_char = string{};
        for_each(begin(input_str), end(input_str), [&mac_addr_hex_chars, &hex_char](const auto &single_char) {
            if (!hex_char.empty()) {
                mac_addr_hex_chars.push_back(hex_char + single_char);
                hex_char.clear();
            } else {
                hex_char.push_back(single_char);
            }
        });
    }

    // if the number of elements are still not == 6 then we have malformed input
    if (mac_addr_hex_chars.size() != MAC_ADDRESS_SIZE) {
        auto error_str = format("mac address is not the correct length (size={} != 6)", mac_addr_hex_chars.size());
        PLOG_ERROR << error_str;
        throw std::invalid_argument(error_str);
    }

    // convert each two character element into an unsigned long integer and store it in the raw mac address
    auto raw_mac_addr = vector<uint8_t>(MAC_ADDRESS_SIZE, 0);
    transform(begin(mac_addr_hex_chars),
              end(mac_addr_hex_chars),
              begin(raw_mac_addr),
              [&input_str](const auto &hex_char) {
                  if (!all(hex_char, is_xdigit()) || hex_char.empty() || hex_char.size() > 2) {
                      auto error_str =
                          format("invalid character (\"{}\") found in mac address {}", hex_char, input_str);
                      PLOG_ERROR << error_str;
                      throw std::invalid_argument(error_str);
                  }
                  auto value = std::stoul(hex_char, nullptr, 16);
                  return numeric_cast<uint8_t>(value);
              });

    // finally create a MacAddress instance
    return MacAddress{raw_mac_addr.data(), MAC_ADDRESS_SIZE};
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
    auto devices = DiscoverNetworkDevices();
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

//static optional<NetworkDevices> DiscoverNetworkDevices(const IpAddress &ip_address) {
static optional<NetworkDevices> DiscoverNetworkDevices() {
    return nullopt;
}

#elif defined(__APPLE__)

static bool IsInterfaceActive(const string &name, sa_family_t family) {
    if (family != AF_INET && family != AF_INET6) {
        PLOG_ERROR << format("only able to open sockets to INET & INET6 where as this is {}", family);
        return false;
    }
    auto sock = socket(family, SOCK_DGRAM, 0);
    if (sock < 0) {
        PLOG_ERROR << format("opening socket failed, family: {}, type: SOCK_DGRAM, protocol: 0, details: {}",
                             family,
                             mmotd::error::posix_error::to_string());
        return false;
    }
    auto socket_closer = sg::make_scope_guard([sock]() { close(sock); });

    auto ifmr = ifmediareq{};
    (void)memset(&ifmr, 0, sizeof(ifmr));
    (void)strncpy(ifmr.ifm_name, name.c_str(), sizeof(ifmr.ifm_name));

    if (ioctl(sock, SIOCGIFMEDIA, (caddr_t)&ifmr) < 0) {
        PLOG_ERROR << format("iterface does not support SIOCGIFMEDIA, details: {}",
                             mmotd::error::posix_error::to_string());
        return false;
    }

    if (ifmr.ifm_count == 0) {
        PLOG_DEBUG << format("ioctl SIOCGIFMEDIA returned no ifmediareq for {}", name);
        return false;
    }

    int *media_list = (int *)malloc(ifmr.ifm_count * sizeof(int));
    if (media_list == NULL) {
        PLOG_ERROR << format("malloc failed to allocate {} bytes", ifmr.ifm_count * sizeof(int));
        return false;
    }
    auto media_list_deleter = sg::make_scope_guard([media_list]() { free(media_list); });
    ifmr.ifm_ulist = media_list;

    if (ioctl(sock, SIOCGIFMEDIA, (caddr_t)&ifmr) < 0) {
        PLOG_ERROR << format("ioctl SIOCGIFMEDIA failed, details: {}", mmotd::error::posix_error::to_string());
        return false;
    }

    auto active = (ifmr.ifm_status & IFM_AVALID) != 0 && (ifmr.ifm_status & IFM_ACTIVE) != 0;
    PLOG_INFO << format("interface {}: {}active", name, active ? "" : "in");

    return active;
}

static optional<NetworkDevices> DiscoverNetworkDevices() {
    struct ifaddrs *addrs = nullptr;
    if (getifaddrs(&addrs) != 0) {
        PLOG_ERROR << format("getifaddrs failed, {}", mmotd::error::posix_error::to_string());
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
        //auto flags = ptr->ifa_flags;
        const auto interface_name = string{ptr->ifa_name};
        const auto address_family = ptr->ifa_addr->sa_family;
        if (address_family == AF_LINK) {
            const auto *sock_addr = reinterpret_cast<const struct sockaddr_dl *>(ptr->ifa_addr);
            const auto ntoa_str = string(link_ntoa(sock_addr));
            const auto index = ntoa_str.find(':');
            if (index == string::npos) {
                PLOG_DEBUG << format("{} found mac address {} which is not of the form interface:xx.xx.xx.xx.xx.xx",
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
                PLOG_DEBUG << format("{} with mac address {} is being replaced with {}",
                                     interface_name,
                                     network_device->mac_address.to_string(),
                                     mac_address);
            }
            network_device->mac_address = MacAddress::from_string(mac_address);
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
            PLOG_DEBUG << format("{} found ip {} for family {}", interface_name, ip_str, family_name);
            auto iter = network_devices.find(interface_name);
            NetworkDevice *network_device = iter == end(network_devices) ? nullptr : &(iter->second);
            if (iter == end(network_devices)) {
                iter = network_devices.insert(cbegin(network_devices), {interface_name, NetworkDevice{}});
                network_device = &(iter->second);
                network_device->interface_name = interface_name;
            }
            PLOG_DEBUG << format("has network device {} been checked for active status? {} ",
                                 interface_name,
                                 boost::indeterminate(network_device->active) ? "no" :
                                 static_cast<bool>(network_device->active)    ? "yes, active{true}" :
                                                                                "yes, active{false}");
            if (boost::indeterminate(network_device->active)) {
                auto active = IsInterfaceActive(interface_name, address_family);
                network_device->active = active;
            }

            auto ip_address = make_address(ip_str);
            if (ip_address.is_unspecified()) {
                PLOG_DEBUG << format("{} with ip address {} is unspecified (bad)",
                                     interface_name,
                                     ip_address.to_string());
                continue;
            }
            if (ip_address.is_loopback()) {
                PLOG_DEBUG << format("{} with ip address {} is a loopback device",
                                     interface_name,
                                     ip_address.to_string());
                continue;
            }
            if (ip_address.is_multicast()) {
                // not a problem that we won't add the ip address
                PLOG_DEBUG << format("{} with ip address {} is a multicast device",
                                     interface_name,
                                     ip_address.to_string());
            }
            if (ip_address.is_v6()) {
                // Not displaying IPv6 addresses at this point
                continue;
            }

            PLOG_DEBUG << format("adding ip address {} to the network device: {}",
                                 ip_address.to_string(),
                                 interface_name);

            network_device->ip_addresses.push_back(ip_address);
        }
    }

    auto cleaned_network_devices = NetworkDevices{};
    copy_if(begin(network_devices),
            end(network_devices),
            inserter(cleaned_network_devices, cleaned_network_devices.begin()),
            [](const auto &device) {
                //[active_ip_address](const auto &device) {
                const auto &network_device = device.second;
                if (network_device.interface_name.empty()) {
                    return false;
                } else if (!boost::indeterminate(network_device.active) && !static_cast<bool>(network_device.active)) {
                    return false;
                } else if (!network_device.mac_address) {
                    return false;
                } else if (network_device.ip_addresses.empty()) {
                    return false;
                } else {
                    auto i = find_if(begin(network_device.ip_addresses),
                                     end(network_device.ip_addresses),
                                     [](const auto &ip_address) { return ip_address.is_v4(); });
                    if (i == end(network_device.ip_addresses)) {
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

    auto buffer = vector<char>(64, 0);
    const char *address_str = inet_ntop(AF_INET, &name.sin_addr, buffer.data(), buffer.size());
    if (address_str == nullptr) {
        PLOG_ERROR << format("inet_ntop failed after connecting to {} on port {}, details: {}",
                             GOOGLE_DNS_IP,
                             DNS_PORT,
                             mmotd::error::posix_error::to_string());
        return nullopt;
    }
    PLOG_DEBUG << format("found active interface {} by connecting to {} on port {}",
                         address_str,
                         GOOGLE_DNS_IP,
                         DNS_PORT);
    auto ip_address = make_address(address_str);
    return make_optional(ip_address);
}

} // namespace mmotd
