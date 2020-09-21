// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "external_network.h"
#include "http_request.h"

#include <json/json.h>
#include <iostream>
#include <sstream>
#include <tuple>

using boost::asio::ip::make_address;
using namespace std;

string ExternalNetwork::to_string() const {
    return ip_address_.to_string();
}

IpAddress ExternalNetwork::ParseHttpResult(const string &response) {
    auto builder = Json::CharReaderBuilder{};
    builder["collectComments"] = false;
    auto value = Json::Value{};
    auto errs = string{};
    auto input_str_stream = istringstream{response};
    auto input_stream = istream{input_str_stream.rdbuf()};
    bool status = Json::parseFromStream(builder, input_stream, &value, &errs);
    if (!status) {
        if (!errs.empty()) {
            cerr << "ERROR: converting HTTP response from ipinfo to JSON. " << errs << "\n";
        } else {
            cerr << "ERROR: converting HTTP response from ipinfo to JSON\n";
        }
        return IpAddress{};
    } else if (value.empty()) {
        cerr << "ERROR: converting HTTP response from ipinfo to JSON. Conversion succeeded but the value is empty\n";
    }

    if (value.isObject() && value.isMember("ip") && value["ip"].isString()) {
        return make_address(value["ip"].asString());
    }
    return IpAddress{};
}

bool ExternalNetwork::TryDiscovery() {
    auto request = HttpRequest{"ipinfo.io"};
    const auto response = request.MakeRequest("/json?token=YOUR_TOKEN_HERE");
    if (response.empty()) {
        cerr << "ERROR: querying 'http://ipinfo.io/json' failed\n";
        return false;
    }
    const auto ip_address = ParseHttpResult(response);
    if (!ip_address.is_unspecified()) {
        ip_address_ = ip_address;
    }
    return !ip_address.is_unspecified();
}

