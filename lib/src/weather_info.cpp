// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/chrono_io.h"
#include "common/include/config_options.h"
#include "common/include/logging.h"
#include "lib/include/computer_information.h"
#include "lib/include/http_request.h"
#include "lib/include/weather_info.h"

#include <iterator>
#include <regex>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

using fmt::format;
using namespace std;
using namespace std::string_literals;

bool gLinkWeatherInfo = false;

namespace {

pair<string, string> ParseSunriseSunset(string sunrise_str, string sunset_str) {
    auto sunrise_holder = mmotd::chrono::io::from_string(sunrise_str, "%H:%M:%S");
    auto sunrise_result = string{};
    if (sunrise_holder) {
        sunrise_result = mmotd::chrono::io::to_string(*sunrise_holder, "%I:%M:%S%p");
    }
    auto sunset_holder = mmotd::chrono::io::from_string(sunset_str, "%H:%M:%S");
    auto sunset_result = string{};
    if (sunset_holder) {
        sunset_result = mmotd::chrono::io::to_string(*sunset_holder, "%I:%M:%S%p");
    }
    return {sunrise_result, sunset_result};
}

string GetLocation(string seperator) {
    using namespace mmotd::core;
    auto city = ConfigOptions::Instance().GetValueAsStringOr("city"s, std::string{});
    auto state = ConfigOptions::Instance().GetValueAsStringOr("state"s, std::string{});
    auto country = ConfigOptions::Instance().GetValueAsStringOr("country"s, std::string{});
    return boost::join_if(vector{city, state, country}, seperator, [](const auto &str) { return !empty(str); });
}

string CreateWeatherRequestUrl() {
    auto location = GetLocation("%20"s);
    // "http://wttr.in/?u&format=%l:+%t+%c+%C+%w+%m+%S+%s&lang=en"
    // "http://wttr.in/Albuquerque%20NM%20USA?u&format=%l:+%t+%c+%C+%w+%m+%S+%s&lang=en"
    return format(FMT_STRING("/{}?u&format=%l:+%t+%c+%C+%w+%m+%S+%s&lang=en"), location);
}

bool IsWeatherResponseInvalid(const string &response) {
    // An html response is an indicator that the response is invalid.
    return boost::icontains(response, "<title>"s);
}

} // namespace

namespace mmotd::information {

static const bool users_logged_in_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::WeatherInfo>(); });

void WeatherInfo::FindInformation() {
    auto [location_str, weather_str, sunrise_str, sunset_str] = GetWeatherInfo();
    if (empty(weather_str)) {
        return;
    }

    auto weather = GetInfoTemplate(InformationId::ID_WEATHER_WEATHER);
    weather.SetValueArgs(weather_str);
    AddInformation(weather);

    if (!empty(sunrise_str)) {
        auto sunrise = GetInfoTemplate(InformationId::ID_WEATHER_SUNRISE);
        sunrise.SetValueArgs(sunrise_str);
        AddInformation(sunrise);
    }

    if (!empty(sunset_str)) {
        auto sunset = GetInfoTemplate(InformationId::ID_WEATHER_SUNSET);
        sunset.SetValueArgs(sunset_str);
        AddInformation(sunset);
    }

    if (empty(location_str)) {
        return;
    }

    auto location = GetInfoTemplate(InformationId::ID_WEATHER_LOCATION);
    location.SetValueArgs(location_str);
    AddInformation(location);
}

tuple<string, string, string, string> WeatherInfo::GetWeatherInfo() {
    using boost::trim_copy, boost::replace_all_copy;
    using namespace mmotd::chrono::io;
    using namespace mmotd::networking;

    auto http_request = HttpRequest(HttpProtocol::HTTPS, "wttr.in");
    auto http_response = http_request.MakeRequest(CreateWeatherRequestUrl());
    if (!http_response || IsWeatherResponseInvalid(*http_response)) {
        return make_tuple(string{}, string{}, string{}, string{});
    }

    auto location_str = GetLocation(" "s);
    auto weather_str = *http_response;
    if (auto i = weather_str.find_first_of(':'); i != string::npos) {
        location_str = boost::trim_copy(weather_str.substr(0, i));
        weather_str = boost::trim_copy(weather_str.substr(i + 1));
    } else {
        LOG_ERROR("weather response appears to be malformed");
        return make_tuple(location_str, weather_str, string{}, string{});
    }

    const auto sunrise_sunset_regex = regex(R"((\d{2}:\d{2}:\d{2}) (\d{2}:\d{2}:\d{2}))", std::regex::ECMAScript);
    auto match = smatch{};
    if (!regex_search(weather_str, match, sunrise_sunset_regex)) {
        LOG_ERROR("weather information does not seem to include sunrise and sunset");
        return make_tuple(location_str, weather_str, string{}, string{});
    }

    auto weather = trim_copy(weather_str.substr(0, match.position(0)));
    LOG_INFO("weather: '{}'", weather);
    auto sunrise_str = weather_str.substr(match.position(1), match.length(1));
    LOG_INFO("sunrise: '{}'", sunrise_str);
    auto sunset_str = weather_str.substr(match.position(2), match.length(2));
    LOG_INFO("sunset: '{}'", sunset_str);

    auto [sunrise_parsed, sunset_parsed] = ParseSunriseSunset(sunrise_str, sunset_str);
    sunrise_parsed = empty(sunrise_parsed) ? sunrise_str : sunrise_parsed;
    sunset_parsed = empty(sunset_parsed) ? sunset_str : sunset_parsed;

    LOG_INFO("weather: '{}, Sunrise (parsed): {}, Sunset (parsed): {}'", weather, sunrise_parsed, sunset_parsed);
    return make_tuple(location_str, weather, sunrise_parsed, sunset_parsed);
}

} // namespace mmotd::information
