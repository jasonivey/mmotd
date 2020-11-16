// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/external_network.h"
#include "lib/include/http_request.h"

#include <plog/Log.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <json/json.h>
#include <sstream>
#include <tuple>

using boost::asio::ip::make_address;
using namespace std;

string ExternalNetwork::to_string() const {
    return ip_address_.to_string();
}

#if 0
static void walk_ptree(const pt::ptree &tree, size_t indent) {
    const string data = tree.data();
    cout << fmt::format("{}data: \"{}\"\n", string(indent, ' '), data);
    for (auto i = begin(tree); i != end(tree); ++i) {
        const auto &child = *i;
        cout << fmt::format("{}child: \"{}\"\n", string(indent, ' '), child.first);
        walk_ptree(child.second, indent + 2);
    }
}
#endif

IpAddress ExternalNetwork::ParseJsonResponse(const string &response) {
    namespace pt = boost::property_tree;
    auto input_str_stream = istringstream{response};
    auto input_stream = istream{input_str_stream.rdbuf()};
    auto tree = pt::ptree{};
    pt::read_json(input_stream, tree);

    //walk_ptree(tree, 2);
    if (tree.empty()) {
        PLOG_ERROR << "http response is empty after converting to json";
    } else {
        auto ip_address_value = tree.get_optional<string>("ip");
        if (ip_address_value) {
            PLOG_INFO << "found ip address in json response body=" << *ip_address_value << "\n";
            return make_address(*ip_address_value);
        }
    }
    return IpAddress{};
}

bool ExternalNetwork::TryDiscovery() {
    auto request = HttpRequest{HttpProtocol::HTTPS, "ipinfo.io"};
    // const auto response = request.MakeRequest("/json?token=YOUR_TOKEN_HERE");
    const auto response = request.MakeRequest("/json");
    if (response.empty()) {
        PLOG_ERROR << "querying 'http://ipinfo.io/json' failed";
        return false;
    }
    const auto ip_address = ParseJsonResponse(response);
    if (!ip_address.is_unspecified()) {
        ip_address_ = ip_address;
    }
    return !ip_address.is_unspecified();
}
