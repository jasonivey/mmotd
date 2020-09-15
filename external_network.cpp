// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "external_network.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <tuple>
#include <fmt/format.h>

using boost::asio::ip::make_address;
using namespace fmt;
using namespace std;

string ExternalNetwork::to_string() const {
    return ip_address_.to_string();
}

static tuple<bool, IpAddress>  DiscoverExternalIpAddress() {
    static constexpr size_t BUFFER_SIZE = max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN) + 1;
    array<char, BUFFER_SIZE> buffer = {0};

    int socket_handle = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_handle == -1) {
        cerr << format("ERROR: unable to open a socket handle, errno: {}\n", errno);
        return make_tuple(false, IpAddress{});
    }

    const char* GOOGLE_DNS_IP = "8.8.8.8";
    static constexpr uint16_t DNS_PORT = 53;

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(GOOGLE_DNS_IP);
    serv.sin_port = htons(DNS_PORT);

    int err = connect(socket_handle, reinterpret_cast<const sockaddr*>(&serv), sizeof(serv));
    if (err == -1) {
        cerr << format("ERROR: connecting to {}:{}, errno: {}\n", GOOGLE_DNS_IP, DNS_PORT, errno);
        return make_tuple(false, IpAddress{});
    }

    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(socket_handle, reinterpret_cast<sockaddr *>(&name), &namelen);
    if (err == -1) {
        cerr << format("ERROR: get socket name after connecting to {}:{}, errno: {}\n", GOOGLE_DNS_IP, DNS_PORT, errno);
        return make_tuple(false, IpAddress{});
    }

    const char *ip_str = inet_ntop(AF_INET, &name.sin_addr, buffer.data(), BUFFER_SIZE);
    if (ip_str == nullptr) {
        cerr << format("ERROR: inet_ntop failed after connecting to {}:{}, errno: {}\n", GOOGLE_DNS_IP, DNS_PORT, errno);
        return make_tuple(false, IpAddress{});
    }
    auto ip_address = make_address(ip_str);
    close(socket_handle);
    return make_tuple(true, ip_address);
}

bool ExternalNetwork::TryDiscovery() {
    const auto [success, ip_address] = DiscoverExternalIpAddress();
    if (success) {
        ip_address_= ip_address;
    }
    return success;
}
