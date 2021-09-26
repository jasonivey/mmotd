// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__linux__)
#include "common/assertion/include/precondition.h"
#include "common/include/logging.h"
#include "common/include/mac_address.h"
#include "common/include/posix_error.h"
#include "lib/include/platform/network.h"

#include <cstring>
#include <optional>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/asio/ip/address.hpp>
#include <fmt/format.h>
#include <scope_guard.hpp>

#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <linux/if_packet.h>

using boost::asio::ip::address;
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
        LOG_ERROR("opening socket failed, family: AF_INET, type: SOCK_DGRAM, protocol: 0, details: {}",
                  mmotd::error::posix_error::to_string());
        return;
    };
    auto socket_closer = sg::make_scope_guard([sock]() noexcept { close(sock); });

    for (auto &device : devices) {
        struct ifreq req {};
        memset(req.ifr_ifrn.ifrn_name, 0, IFNAMSIZ);
        memcpy(req.ifr_ifrn.ifrn_name,
               data(device.interface_name),
               min(size(device.interface_name), static_cast<size_t>(IFNAMSIZ - 1)));

        if (ioctl(sock, SIOCGIFFLAGS, &req) == -1) {
            LOG_ERROR("ioctl SIOCGIFFLAGS failed on interface {}, details: {}",
                      device.interface_name,
                      mmotd::error::posix_error::to_string());
            return;
        }

        int flags = req.ifr_ifru.ifru_flags;
        auto active_status = (flags & IFF_UP) != 0;
        LOG_VERBOSE("interface: {}, active status: {}", device.interface_name, active_status ? "active" : "inactive");
        device.active = active_status;
    }
}

bool IsMacAddressFamily(sa_family_t address_family) {
    return address_family == AF_PACKET;
}

bool IsIpv4AddressFamily(sa_family_t address_family) {
    return address_family == AF_INET;
}

bool IsIpv6AddressFamily(sa_family_t address_family) {
    return address_family == AF_INET6;
}

void AddMacAddress(NetworkDevices &network_devices, const struct ifaddrs *ifaddrs_ptr) {
    PRECONDITIONS(ifaddrs_ptr != nullptr, "input ifaddrs must be valid");

    const auto interface_name = string{ifaddrs_ptr->ifa_name};
    const auto *sock_addr = reinterpret_cast<const struct sockaddr_ll *>(ifaddrs_ptr->ifa_addr);
    auto mac_address = MacAddress{sock_addr->sll_addr, sock_addr->sll_halen};
    LOG_VERBOSE("{} found mac address {}", interface_name, mac_address.to_string());
    network_devices.AddMacAddress(interface_name, mac_address);
}

optional<address> CreateIpAddress(const string &ip_str, const string &interface_name) {
    PRECONDITIONS(!empty(ip_str), "unable to create ip address from an empty string");
    PRECONDITIONS(!empty(interface_name), "unable to add ip address from an empty interface name");

    auto ec = boost::system::error_code{};
    auto ip_address = make_address(ip_str, ec);
    if (ec) {
        LOG_ERROR("creating address '{}' for interface '{}', error {}: {}",
                  ip_str,
                  interface_name,
                  ec.value(),
                  ec.message());
        return nullopt;
    } else if (ip_address.is_unspecified()) {
        LOG_VERBOSE("{} with ip address {} is unspecified (bad)", interface_name, ip_address.to_string());
        return nullopt;
    } else if (ip_address.is_loopback()) {
        LOG_VERBOSE("{} with ip address {} is a loopback device", interface_name, ip_address.to_string());
        return nullopt;
    } else if (ip_address.is_v6()) {
        LOG_VERBOSE("{} with v6 ip address {} -- ignoring ipv6 addresses", interface_name, ip_address.to_string());
        return nullopt;
    } else if (ip_address.is_multicast()) {
        // just noting this property -- it doesn't stop us from adding the device
        LOG_VERBOSE("{} with ip address {} is a multicast device", interface_name, ip_address.to_string());
    }
    return {ip_address};
}

void AddIpAddress(NetworkDevices &network_devices, const struct ifaddrs *ifaddrs_ptr) {
    PRECONDITIONS(ifaddrs_ptr != nullptr, "input ifaddrs must be valid");

    const auto interface_name = string{ifaddrs_ptr->ifa_name};
    const auto address_family = ifaddrs_ptr->ifa_addr->sa_family;

    static constexpr size_t BUFFER_SIZE = max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN) + 1;
    auto buffer = array<char, BUFFER_SIZE>{0};

    auto ip_str = string{};
    if (address_family == AF_INET) {
        const auto *sock_addr = reinterpret_cast<struct sockaddr_in *>(ifaddrs_ptr->ifa_addr);
        if (inet_ntop(AF_INET, &sock_addr->sin_addr, buffer.data(), INET_ADDRSTRLEN)) {
            ip_str = string(buffer.data());
        }
    } else {
        const auto *sock_addr = reinterpret_cast<struct sockaddr_in6 *>(ifaddrs_ptr->ifa_addr);
        if (inet_ntop(AF_INET6, &sock_addr->sin6_addr, buffer.data(), INET6_ADDRSTRLEN)) {
            ip_str = string(buffer.data());
        }
    }

    LOG_DEBUG("{} found ip {} for family {}", interface_name, ip_str, address_family == AF_INET ? "IPv4" : "IPv6");
    auto ip_address_holder = CreateIpAddress(ip_str, interface_name);
    if (ip_address_holder) {
        auto ip_address = *ip_address_holder;
        LOG_VERBOSE("adding ip address {} to the network device: {}", ip_address.to_string(), interface_name);
        network_devices.AddIpAddress(interface_name, ip_address);
    }
}

} // namespace

namespace mmotd::platform {

NetworkDevices GetNetworkDevices() {
    struct ifaddrs *addrs = nullptr;
    if (getifaddrs(&addrs) != 0) {
        LOG_ERROR("getifaddrs failed, {}", mmotd::error::posix_error::to_string());
        return NetworkDevices{};
    }

    auto freeifaddrs_deleter = sg::make_scope_guard([addrs]() noexcept { freeifaddrs(addrs); });

    auto network_devices = NetworkDevices{};
    for (struct ifaddrs *ptr = addrs; ptr; ptr = ptr->ifa_next) {
        if (ptr->ifa_addr == nullptr || ptr->ifa_name == nullptr) {
            continue;
        }

        const auto interface_name = string{ptr->ifa_name};
        const auto address_family = ptr->ifa_addr->sa_family;
        network_devices.AddInterface(interface_name);

        if (IsMacAddressFamily(address_family)) {
            AddMacAddress(network_devices, ptr);
        } else if (IsIpv4AddressFamily(address_family) || IsIpv6AddressFamily(address_family)) {
            AddIpAddress(network_devices, ptr);
        }
    }

    SetActiveInterfaces(network_devices);
    network_devices.FilterWorthless();
    return network_devices;
}

} // namespace mmotd::platform
#endif
