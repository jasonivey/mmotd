// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/network_device.h"
#include "common/include/posix_error.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <iterator>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/asio/ip/address.hpp>
#include <fmt/format.h>
#include <plog/Log.h>
#include <scope_guard.hpp>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if_dl.h>
#include <net/if_media.h>
#include <netinet/in.h>
#include <netinet6/in6_var.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using boost::asio::ip::make_address;
using fmt::format;
using namespace std;
using mmotd::networking::IpAddress;
using mmotd::networking::MacAddress;
using mmotd::networking::NetworkDevices;

namespace {

bool IsInterfaceActive(const string &name, sa_family_t family) {
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
    auto socket_closer = sg::make_scope_guard([sock]() noexcept { close(sock); });

    auto ifmr = ifmediareq{};
    memset(&ifmr, 0, sizeof(ifmr));
    memcpy(ifmr.ifm_name, data(name), min(size(name), static_cast<size_t>(IFNAMSIZ - 1)));

    if (ioctl(sock, SIOCGIFMEDIA, reinterpret_cast<caddr_t>(&ifmr)) < 0) {
        PLOG_ERROR << format("iterface does not support SIOCGIFMEDIA, details: {}",
                             mmotd::error::posix_error::to_string());
        return false;
    }

    if (ifmr.ifm_count == 0) {
        PLOG_DEBUG << format("ioctl SIOCGIFMEDIA returned no ifmediareq for {}", name);
        return false;
    }

    int *media_list = static_cast<int *>(malloc(static_cast<size_t>(ifmr.ifm_count) * sizeof(int)));
    if (media_list == NULL) {
        PLOG_ERROR << format("malloc failed to allocate {} bytes", static_cast<size_t>(ifmr.ifm_count) * sizeof(int));
        return false;
    }

    auto media_list_deleter = sg::make_scope_guard([media_list]() noexcept { free(media_list); });
    ifmr.ifm_ulist = media_list;

    if (ioctl(sock, SIOCGIFMEDIA, reinterpret_cast<caddr_t>(&ifmr)) < 0) {
        PLOG_ERROR << format("ioctl SIOCGIFMEDIA failed, details: {}", mmotd::error::posix_error::to_string());
        return false;
    }

    auto active = (ifmr.ifm_status & IFM_AVALID) != 0 && (ifmr.ifm_status & IFM_ACTIVE) != 0;
    PLOG_INFO << format("interface {}: {}active", name, active ? "" : "in");

    return active;
}

} // namespace

namespace mmotd::platform {

NetworkDevices GetNetworkDevices() {
    struct ifaddrs *addrs = nullptr;
    if (getifaddrs(&addrs) != 0) {
        PLOG_ERROR << format("getifaddrs failed, {}", mmotd::error::posix_error::to_string());
        return NetworkDevices{};
    }
    auto freeifaddrs_deleter = sg::make_scope_guard([addrs]() noexcept { freeifaddrs(addrs); });

    auto network_devices = NetworkDevices{};
    static constexpr size_t BUFFER_SIZE = max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN) + 1;
    array<char, BUFFER_SIZE> buffer = {0};
    for (struct ifaddrs *ptr = addrs; ptr; ptr = ptr->ifa_next) {
        if (ptr->ifa_addr == nullptr || ptr->ifa_name == nullptr) {
            continue;
        }
        const auto interface_name = string{ptr->ifa_name};
        const auto address_family = ptr->ifa_addr->sa_family;
        network_devices.AddInterface(interface_name);
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
            auto mac_address = ntoa_str.substr(index + 1);
            network_devices.AddMacAddress(interface_name, MacAddress::from_string(mac_address));
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
            PLOG_DEBUG << format("has network device {} been checked for active status? {} ",
                                 interface_name,
                                 network_devices.HasActiveBeenTested(interface_name) ? "no" :
                                 network_devices.IsActive(interface_name)            ? "yes, {active}" :
                                                                                       "yes, {inactive}");
            if (!network_devices.HasActiveBeenTested(interface_name)) {
                auto active_status = IsInterfaceActive(interface_name, address_family);
                network_devices.SetActive(interface_name, active_status);
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

            network_devices.AddIpAddress(interface_name, ip_address);
        }
    }

    network_devices.FilterWorthless();
    return network_devices;
}

} // namespace mmotd::platform
