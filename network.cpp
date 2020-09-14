#include "network.h"
#if 0
#include <sys/socket.h>
#include <net/if.h>
#include <net/ndrv.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/ethernet.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <string.h>
//#include <sys/ioctl.h>
//#include <net/if.h>
//#include <arpa/inet.h>

#include <string>
#include <iostream>

using namespace std;

#if defined(__linux__)
bool get_mac_address(char* mac_addr, const char* if_name = "eth0")
{
    struct ifreq ifinfo;
    strcpy(ifinfo.ifr_name, if_name);
    int sd = socket(AF_INET, SOCK_DGRAM, 0);
    int result = ioctl(sd, SIOCGIFHWADDR, &ifinfo);
    close(sd);

    if ((result == 0) && (ifinfo.ifr_hwaddr.sa_family == 1)) {
        memcpy(mac_addr, ifinfo.ifr_hwaddr.sa_data, IFHWADDRLEN);
        return true;
    }
    else {
        return false;
    }
}
#elif defined(__APPLE__)
bool NetworkInfo::QueryForNetworkInfo() {
    struct ifaddrs* addrs = nullptr;
    if (getifaddrs(&addrs) != 0) {
        cerr << "ERROR: getifaddrs failed\n";
        return false;
    }

    auto network_device = NetworkDevice{};
    for (struct ifaddrs* ptr = addrs; ptr; ptr = ptr->ifa_next) {
        if (ptr->ifa_addr == nullptr) {
            continue;
        }
        const auto address_family = ptr->ifa_addr->sa_family;
        if (address_family == AF_LINK) {
            const auto *sock_addr = reinterpret_cast<const struct sockaddr_dl *>(ptr->ifa_addr);
            auto ntoa_str = string(link_ntoa(sock_addr));
            auto index = ntoa_str.find(':');
            if (index == string::npos) {
                continue;
            }
            auto interface_name = ntoa_str.substr(0, index);
            auto mac_address = ntoa_str.substr(index + 1);
            network_device.interface_name = interface_name;
            network_device.mac_address = MacAddress(mac_address);
            cout << "interface   : " << interface_name << endl;
            cout << "mac address : " << mac_address << endl << endl;
        }
        else if (address_family == AF_INET || address_family == AF_INET6)
        {
            const auto *sock_addr = reinterpret_cast<struct sockaddr_in *>(ptr->ifa_addr);
            auto retptr = inet_ntoa(sock_addr->sin_addr);
            if (retptr == nullptr) {
                cerr << "ERROR: calling inet_ntop -- errno: " << errno << "\n";
                continue;
            }
            cout << "raw ip      : " << string{retptr} << endl;
            auto ip_addr = boost::asio::ip::make_address(retptr);
            auto interface_name = string{ptr->ifa_name};
            cout << "interface   : " << interface_name << endl;
            cout << "ip address  : " << ip_addr.to_string() << endl;
            cout << "ip unspecified : " << std::boolalpha << ip_addr.is_unspecified() << endl << endl;
        }
    }
    freeifaddrs(addrs);
    return true;
}
#else
#   error no definition for get_mac_address() on this platform!
#endif
