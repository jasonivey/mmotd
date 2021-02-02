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

namespace mmotd::information {

static const bool external_network_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::ExternalNetwork>(); });

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

// const auto response = request.MakeRequest("/json?token=YOUR_TOKEN_HERE");
bool ExternalNetwork::FindInformation() {
    using mmotd::networking::HttpRequest, mmotd::networking::HttpProtocol;

    if (const auto response = HttpRequest{HttpProtocol::HTTPS, "ipinfo.io"}.MakeRequest("/json"); response) {
        return ParseJsonResponse(*response);
    } else {
        PLOG_ERROR << "querying 'http://ipinfo.io/json' failed";
        return false;
    }
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
    if (auto ip_address_value = tree.get_optional<string>("ip"); ip_address_value) {
        PLOG_DEBUG << format("found ip address: {} in json response body", *ip_address_value);
        auto ip_address = make_address(*ip_address_value);
        auto ip = GetInfoTemplate(InformationId::ID_EXTERNAL_NETWORK_INFO_EXTERNAL_IP);
        ip.information = ip_address.to_string();
        AddInformation(ip);
        retval = true;
    }
    if (auto city_value = tree.get_optional<string>("city"); city_value) {
        PLOG_DEBUG << format("found city: {} in json response body", *city_value);
        auto city = GetInfoTemplate(InformationId::ID_LOCATION_INFO_CITY);
        city.information = *city_value;
        AddInformation(city);
    }
    if (auto country_value = tree.get_optional<string>("country"); country_value) {
        PLOG_DEBUG << format("found country: {} in json response body", *country_value);
        auto country = GetInfoTemplate(InformationId::ID_LOCATION_INFO_COUNTRY);
        country.information = *country_value;
        AddInformation(country);
    }
    if (auto gps_location_value = tree.get_optional<string>("loc"); gps_location_value) {
        PLOG_DEBUG << format("found gps location: {} in json response body", *gps_location_value);
        auto gps = GetInfoTemplate(InformationId::ID_LOCATION_INFO_GPS_LOCATION);
        gps.information = *gps_location_value;
        AddInformation(gps);
    }
    if (auto zip_code_value = tree.get_optional<string>("postal"); zip_code_value) {
        PLOG_DEBUG << format("found zip code: {} in json response body", *zip_code_value);
        auto zipcode = GetInfoTemplate(InformationId::ID_LOCATION_INFO_ZIP_CODE);
        zipcode.information = *zip_code_value;
        AddInformation(zipcode);
    }
    if (auto state_value = tree.get_optional<string>("region"); state_value) {
        PLOG_DEBUG << format("found state: {} in json response body", *state_value);
        auto state = GetInfoTemplate(InformationId::ID_LOCATION_INFO_STATE);
        state.information = *state_value;
        AddInformation(state);
    }
    if (auto timezone_value = tree.get_optional<string>("timezone"); timezone_value) {
        PLOG_DEBUG << format("found timezone: {} in json response body", *timezone_value);
        auto tz = GetInfoTemplate(InformationId::ID_LOCATION_INFO_TIMEZONE);
        tz.information = *timezone_value;
        AddInformation(tz);
    }
    return retval;
}

} // namespace mmotd::information
