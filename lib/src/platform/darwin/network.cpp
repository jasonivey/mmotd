// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/network_device.h"
#include "common/include/posix_error.h"

#include <algorithm>
#include <cstdio>
#include <iterator>
#include <optional>
#include <stdexcept>
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

#if 0
optional<IpAddress> GetActiveInterface() {
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
#endif

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
        //auto flags = ptr->ifa_flags;
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
