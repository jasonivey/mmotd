// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/mac_address.h"
#include "common/include/posix_error.h"
#include "lib/include/platform/network.h"

#include <optional>
#include <regex>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <plog/Log.h>
#include <scope_guard.hpp>

#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <linux/if_packet.h>

using boost::asio::ip::make_address;
using fmt::format;
using namespace std;
using mmotd::networking::IpAddress;
using mmotd::networking::MacAddress;
using mmotd::networking::NetworkDevices;

namespace {

void SetActiveInterfaces(NetworkDevices &devices) {
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) {
        PLOG_ERROR << format("opening socket failed, family: AF_PACKET, type: SOCK_DGRAM, protocol: 0, details: {}",
                             mmotd::error::posix_error::to_string());
        return;
    };
    auto socket_closer = sg::make_scope_guard([sock]() { close(sock); });

    for (auto &[name, device] : devices) {
        struct ifreq req {};
        strncpy(req.ifr_ifrn.ifrn_name, name.c_str(), IFNAMSIZ);

        if (ioctl(sock, SIOCGIFFLAGS, &req) == -1) {
            PLOG_ERROR << format("ioctl SIOCGIFFLAGS failed on {}, details: {}",
                                 name,
                                 mmotd::error::posix_error::to_string());
            return;
        }

        int flags = req.ifr_ifru.ifru_flags;
        auto active_status = (flags & IFF_UP) != 0;
        PLOG_VERBOSE << format("{} status: {}", name, active_status ? "active" : "inactive");
        device.active = active_status;
    }
}

} // namespace

namespace mmotd::platform {

NetworkDevices GetNetworkDevices() {
    struct ifaddrs *addrs = nullptr;
    if (getifaddrs(&addrs) != 0) {
        PLOG_ERROR << format("getifaddrs failed, {}", mmotd::error::posix_error::to_string());
        return NetworkDevices{};
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
        network_devices.AddInterface(interface_name);
        if (address_family == AF_PACKET) {
            const auto *sock_addr = reinterpret_cast<struct sockaddr_ll *>(ptr->ifa_addr);
            auto mac_address = MacAddress{sock_addr->sll_addr, sock_addr->sll_halen};
            PLOG_DEBUG << format("{} found mac address {}", interface_name, mac_address.to_string());
            network_devices.AddMacAddress(interface_name, mac_address);
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
            auto ip_address = make_address(ip_str);
            if (ip_address.is_unspecified()) {
                PLOG_DEBUG << format("{} with ip address {} is unspecified (empty)",
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
                continue;
            }
            if (ip_address.is_v6()) {
                // Not displaying IPv6 addresses at this point
                PLOG_DEBUG << format("{} with ip address {} is ipv6 [ignoring]",
                                     interface_name,
                                     ip_address.to_string());
                continue;
            }
            network_devices.AddIpAddress(interface_name, ip_address);
        }
    }

    SetActiveInterfaces(network_devices);
    network_devices.FilterWorthless();
    return network_devices;
}

} // namespace mmotd::platform
