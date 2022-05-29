// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/external_network.h"

#include "common/assertion/include/throw.h"
#include "common/include/logging.h"
#include "common/include/special_files.h"
#include "lib/include/computer_information.h"
#include "lib/include/http_request.h"

#include <sstream>
#include <string_view>
#include <tuple>
#include <utility>

#include <boost/algorithm/string.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fmt/format.h>

using boost::asio::ip::make_address;
using fmt::format;
using namespace std;
using namespace std::string_literals;
using mmotd::core::special_files::GetEnvironmentValue;

bool gLinkExternalNetwork = false;

namespace {

// https://ipinfo.io/json?token=YOUR_TOKEN_HERE
static constexpr auto EXTERNAL_IP_SERVICE_URL = string_view{"ipinfo.io"};

pair<string, string> CreateQueryString() {
    auto api_key = GetEnvironmentValue("$IPINFO_API_KEY");
    string query, log_safe_query;
    if (!empty(api_key)) {
        query = format(FMT_STRING("token={}"), api_key);
        log_safe_query = format(FMT_STRING("token={}"), string(size(api_key), '*'));
    }
    return make_pair(query, log_safe_query);
}

} // namespace

namespace mmotd::information {

static const bool external_network_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::ExternalNetwork>(); });

void ExternalNetwork::FindInformation() {
    using namespace mmotd::networking;
    auto path = "json"sv;
    auto [query, log_safe_query] = CreateQueryString();
    auto response = HttpRequest::Get(HttpProtocol::HTTPS, EXTERNAL_IP_SERVICE_URL, path, query);
    if (response.has_value()) {
        ParseJsonResponse(*response);
    } else {
        auto url = HttpRequest::GetUrl(HttpProtocol::HTTPS, EXTERNAL_IP_SERVICE_URL, path, log_safe_query);
        LOG_ERROR("querying '{}' failed", url);
    }
}

void ExternalNetwork::ParseJsonResponse(const string &response) {
    namespace pt = boost::property_tree;
    auto input_str_stream = istringstream{response};
    auto input_stream = istream{input_str_stream.rdbuf()};
    auto tree = pt::ptree{};

    auto exception_message = string{};
    try {
        pt::read_json(input_stream, tree);
    } catch (boost::exception &ex) {
        exception_message = mmotd::assertion::GetBoostExceptionMessage(ex, "while parsing external network response"sv);
    } catch (const std::exception &ex) {
        exception_message = mmotd::assertion::GetStdExceptionMessage(ex, "while parsing external network response"sv);
    } catch (...) {
        exception_message = mmotd::assertion::GetUnknownExceptionMessage("while parsing external network response"sv);
    }

    if (!empty(exception_message)) {
        LOG_ERROR("{}", exception_message);
        return;
    }

    // walk_ptree(tree, 2);
    if (tree.empty()) {
        LOG_ERROR("http response is empty after converting to json");
    }

    if (auto ip_address_value = tree.get_optional<string>("ip"); ip_address_value) {
        LOG_DEBUG("found ip address: {} in json response body", *ip_address_value);
        auto ec = boost::system::error_code{};
        auto ip_address = make_address(*ip_address_value, ec);
        if (ec) {
            LOG_ERROR("unable to convert '{}' to ip address, {}: {}",
                      *ip_address_value,
                      ec.category().name(),
                      ec.value());
            return;
        }
        auto ip = GetInfoTemplate(InformationId::ID_EXTERNAL_NETWORK_INFO_EXTERNAL_IP);
        ip.SetValueArgs(ip_address.to_string());
        AddInformation(ip);
    }
    if (auto city_value = tree.get_optional<string>("city"); city_value) {
        LOG_DEBUG("found city: {} in json response body", *city_value);
        auto city = GetInfoTemplate(InformationId::ID_LOCATION_INFO_CITY);
        city.SetValueArgs(*city_value);
        AddInformation(city);
    }
    if (auto country_value = tree.get_optional<string>("country"); country_value) {
        LOG_DEBUG("found country: {} in json response body", *country_value);
        auto country = GetInfoTemplate(InformationId::ID_LOCATION_INFO_COUNTRY);
        country.SetValueArgs(*country_value);
        AddInformation(country);
    }
    if (auto gps_location_value = tree.get_optional<string>("loc"); gps_location_value) {
        LOG_DEBUG("found gps location: {} in json response body", *gps_location_value);
        auto gps = GetInfoTemplate(InformationId::ID_LOCATION_INFO_GPS_LOCATION);
        gps.SetValueArgs(*gps_location_value);
        AddInformation(gps);
    }
    if (auto zip_code_value = tree.get_optional<string>("postal"); zip_code_value) {
        LOG_DEBUG("found zip code: {} in json response body", *zip_code_value);
        auto zipcode = GetInfoTemplate(InformationId::ID_LOCATION_INFO_ZIP_CODE);
        zipcode.SetValueArgs(*zip_code_value);
        AddInformation(zipcode);
    }
    if (auto state_value = tree.get_optional<string>("region"); state_value) {
        LOG_DEBUG("found state: {} in json response body", *state_value);
        auto state = GetInfoTemplate(InformationId::ID_LOCATION_INFO_STATE);
        state.SetValueArgs(*state_value);
        AddInformation(state);
    }
    if (auto timezone_value = tree.get_optional<string>("timezone"); timezone_value) {
        LOG_DEBUG("found timezone: {} in json response body", *timezone_value);
        auto tz = GetInfoTemplate(InformationId::ID_LOCATION_INFO_TIMEZONE);
        tz.SetValueArgs(*timezone_value);
        AddInformation(tz);
    }
}

} // namespace mmotd::information
