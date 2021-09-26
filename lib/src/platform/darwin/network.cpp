// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__APPLE__)
#include "common/assertion/include/precondition.h"
#include "common/include/logging.h"
#include "common/include/network_device.h"
#include "common/include/posix_error.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/asio/ip/address.hpp>
#include <fmt/format.h>
#include <scope_guard.hpp>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if_dl.h>
#include <net/if_media.h>
#include <netinet/in.h>
#include <netinet6/in6_var.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using boost::asio::ip::address;
using boost::asio::ip::make_address;
using fmt::format;
using namespace std;
using mmotd::networking::IpAddress;
using mmotd::networking::MacAddress;
using mmotd::networking::NetworkDevices;

namespace {

bool IsInterfaceActive(const string &name, sa_family_t address_family, const string &family_name) {
    if (address_family != AF_INET && address_family != AF_INET6) {
        LOG_ERROR("unable to open sockets to {} for {}", family_name, name);
        return false;
    }
    auto sock = socket(address_family, SOCK_DGRAM, 0);
    if (sock < 0) {
        LOG_ERROR("opening socket failed for {}, family: {}, type: SOCK_DGRAM, protocol: 0, details: {}",
                  name,
                  family_name,
                  mmotd::error::posix_error::to_string());
        return false;
    }
    auto socket_closer = sg::make_scope_guard([sock]() noexcept { close(sock); });

    auto ifmr = ifmediareq{};
    memset(&ifmr, 0, sizeof(ifmr));
    memcpy(ifmr.ifm_name, data(name), min(size(name), static_cast<size_t>(IFNAMSIZ - 1)));

    if (ioctl(sock, SIOCGIFMEDIA, reinterpret_cast<caddr_t>(&ifmr)) < 0) {
        LOG_DEBUG("iterface does not support SIOCGIFMEDIA for {} in {}, details: {}",
                  name,
                  family_name,
                  mmotd::error::posix_error::to_string());
        return false;
    }

    if (ifmr.ifm_count == 0) {
        LOG_DEBUG("ioctl SIOCGIFMEDIA returned no ifmediareq for {} in {}", name, family_name);
        return false;
    }

    auto media_list = vector<int>(static_cast<size_t>(ifmr.ifm_count));
    ifmr.ifm_ulist = media_list.data();

    if (ioctl(sock, SIOCGIFMEDIA, reinterpret_cast<caddr_t>(&ifmr)) < 0) {
        LOG_ERROR("ioctl SIOCGIFMEDIA failed for {} in {}, details: {}",
                  name,
                  family_name,
                  mmotd::error::posix_error::to_string());
        return false;
    }

    auto active = (ifmr.ifm_status & IFM_AVALID) != 0 && (ifmr.ifm_status & IFM_ACTIVE) != 0;
    LOG_VERBOSE("interface {} in {}: found to be {}active", name, family_name, active ? "" : "in");

    return active;
}

bool IsMacAddressFamily(sa_family_t address_family) {
    return address_family == AF_LINK;
}

bool IsIpv4AddressFamily(sa_family_t address_family) {
    return address_family == AF_INET;
}

bool IsIpv6AddressFamily(sa_family_t address_family) {
    return address_family == AF_INET6;
}

void AddMacAddress(NetworkDevices &network_devices, const struct ifaddrs *ifaddrs_ptr) {
    MMOTD_PRECONDITIONS(ifaddrs_ptr != nullptr, "input ifaddrs must be valid");

    const auto interface_name = string{ifaddrs_ptr->ifa_name};
    const auto *sock_addr = reinterpret_cast<const struct sockaddr_dl *>(ifaddrs_ptr->ifa_addr);
    const auto ntoa_str = string(link_ntoa(sock_addr));
    const auto index = ntoa_str.find(':');
    if (index == string::npos) {
        LOG_DEBUG("{} found mac address {} which is not of the form interface:xx.xx.xx.xx.xx.xx",
                  interface_name,
                  ntoa_str);
        return;
    }
    auto mac_address = ntoa_str.substr(index + 1);
    network_devices.AddMacAddress(interface_name, MacAddress::from_string(mac_address));
}

optional<address> CreateIpAddress(const string &ip_str, const string &interface_name) {
    MMOTD_PRECONDITIONS(!empty(ip_str), "unable to create ip address from an empty string");
    MMOTD_PRECONDITIONS(!empty(interface_name), "unable to add ip address from an empty interface name");

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

void SetActiveDevice(NetworkDevices &network_devices,
                     const string &interface_name,
                     sa_family_t address_family,
                     const string &family_name) {
    auto active_been_tested = network_devices.HasActiveBeenTested(interface_name);
    auto active_str = string{};
    if (active_been_tested) {
        if (network_devices.IsActive(interface_name)) {
            active_str = "<tested>, <active>";
        } else {
            active_str = "<tested>, <inactive>";
        }
    } else {
        active_str = "<untested>, <unknown>";
    }
    LOG_DEBUG("has network device {} been checked for active status? {} ", interface_name, active_str);
    if (!active_been_tested) {
        auto active_status = IsInterfaceActive(interface_name, address_family, family_name);
        network_devices.SetActive(interface_name, active_status);
    }
}

void AddIpAddress(NetworkDevices &network_devices, const struct ifaddrs *ifaddrs_ptr) {
    MMOTD_PRECONDITIONS(ifaddrs_ptr != nullptr, "input ifaddrs must be valid");

    static constexpr size_t BUFFER_SIZE = max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN) + 1;
    auto buffer = array<char, BUFFER_SIZE>{0};

    const auto interface_name = string{ifaddrs_ptr->ifa_name};
    const auto address_family = ifaddrs_ptr->ifa_addr->sa_family;

    auto ip_str = string{};
    if (address_family == AF_INET) {
        const auto *sock_addr = reinterpret_cast<struct sockaddr_in *>(ifaddrs_ptr->ifa_addr);
        if (inet_ntop(AF_INET, &sock_addr->sin_addr, data(buffer), INET_ADDRSTRLEN)) {
            ip_str = string(data(buffer));
        }
    } else {
        const auto *sock_addr = reinterpret_cast<struct sockaddr_in6 *>(ifaddrs_ptr->ifa_addr);
        if (inet_ntop(AF_INET6, &sock_addr->sin6_addr, data(buffer), INET6_ADDRSTRLEN)) {
            ip_str = string(data(buffer));
        }
    }

    const auto family_name = address_family == AF_INET ? string{"IPv4"} : string{"IPv6"};
    LOG_DEBUG("{} found ip {} for family {}", interface_name, ip_str, family_name);

    auto ip_address_holder = CreateIpAddress(ip_str, interface_name);
    if (ip_address_holder) {
        auto ip_address = *ip_address_holder;
        LOG_VERBOSE("adding ip address {} to the network device: {}", ip_address.to_string(), interface_name);
        network_devices.AddIpAddress(interface_name, ip_address);
    }

    SetActiveDevice(network_devices, interface_name, address_family, family_name);
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

    network_devices.FilterWorthless();
    return network_devices;
}

} // namespace mmotd::platform
#endif
