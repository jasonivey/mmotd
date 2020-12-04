// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/external_network.h"
#include "lib/include/http_request.h"

#include <sstream>
#include <tuple>

#include <boost/asio/ip/address.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

using boost::asio::ip::make_address;
using fmt::format;
using namespace std;

bool gLinkExternalNetwork = false;

namespace mmotd {

static const bool external_network_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::ExternalNetwork>(); });

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

bool ExternalNetwork::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        return RequestExternalIpAddress();
    }
    return has_queried;
}

optional<mmotd::ComputerValues> ExternalNetwork::GetInformation() const {
    return make_optional(details_);
}

bool ExternalNetwork::RequestExternalIpAddress() {
    // const auto response = request.MakeRequest("/json?token=YOUR_TOKEN_HERE");
    auto request = HttpRequest{HttpProtocol::HTTPS, "ipinfo.io"};
    const auto response = request.MakeRequest("/json");
    if (response.empty()) {
        PLOG_ERROR << "querying 'http://ipinfo.io/json' failed";
        return false;
    }
    return ParseJsonResponse(response);
}

bool ExternalNetwork::ParseJsonResponse(const string &response) {
    namespace pt = boost::property_tree;
    auto input_str_stream = istringstream{response};
    auto input_stream = istream{input_str_stream.rdbuf()};
    auto tree = pt::ptree{};
    pt::read_json(input_stream, tree);

    // walk_ptree(tree, 2);
    if (tree.empty()) {
        PLOG_ERROR << "http response is empty after converting to json";
        return false;
    }
    auto retval = false;
    auto ip_address_value = tree.get_optional<string>("ip");
    if (ip_address_value) {
        PLOG_INFO << format("found ip address: {} in json response body", *ip_address_value);
        auto ip_address = make_address(*ip_address_value);
        details_.push_back(make_tuple("public ip", ip_address.to_string()));
        retval = true;
    }
    auto city_value = tree.get_optional<string>("city");
    if (city_value) {
        PLOG_INFO << format("found city: {} in json response body", *city_value);
        details_.push_back(make_tuple("city", *city_value));
    }
    auto country_value = tree.get_optional<string>("country");
    if (country_value) {
        PLOG_INFO << format("found country: {} in json response body", *country_value);
        details_.push_back(make_tuple("country", *country_value));
    }
    auto gps_location_value = tree.get_optional<string>("loc");
    if (gps_location_value) {
        PLOG_INFO << format("found gps location: {} in json response body", *gps_location_value);
        details_.push_back(make_tuple("gps location", *gps_location_value));
    }
    auto zip_code_value = tree.get_optional<string>("postal");
    if (zip_code_value) {
        PLOG_INFO << format("found zip code: {} in json response body", *zip_code_value);
        details_.push_back(make_tuple("zip code", *zip_code_value));
    }
    auto state_value = tree.get_optional<string>("region");
    if (state_value) {
        PLOG_INFO << format("found state: {} in json response body", *state_value);
        details_.push_back(make_tuple("state", *state_value));
    }
    auto timezone_value = tree.get_optional<string>("timezone");
    if (timezone_value) {
        PLOG_INFO << format("found timezone: {} in json response body", *timezone_value);
        details_.push_back(make_tuple("timezone", *timezone_value));
    }
    return retval;
}

} // namespace mmotd
