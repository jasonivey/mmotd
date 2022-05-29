// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/weather_info.h"

#include "common/assertion/include/assertion.h"
#include "common/include/chrono_io.h"
#include "common/include/config_options.h"
#include "common/include/logging.h"
#include "lib/include/computer_information.h"
#include "lib/include/http_request.h"

#include <iterator>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

using namespace std;
using namespace std::string_literals;

bool gLinkWeatherInfo = false;

namespace {

static constexpr auto WEATHER_SERVICE_URL = string_view{"wttr.in"};
static constexpr auto WEATHER_QUERY = string_view{"u&format=%l:+%t+%c+%C+%w+%m+%S+%s&lang=en"};

// parse input as : "[00-23]h[00-59]m[00-59]s"
// print output as: "[01-12]h[00-59]m[00-59]s[am|pm]"
optional<string> ParseTodPrintTod(string input) {
    using namespace mmotd::chrono::io;
    if (auto time_of_day_holder = time_from_string(input, "%H:%M:%S"); time_of_day_holder.has_value()) {
        return to_string(*time_of_day_holder, "%I:%M:%S%p");
    }
    return nullopt;
}

string GetLocation(string seperator) {
    using namespace mmotd::core;
    auto city = ConfigOptions::Instance().GetString("location.city"s, std::string{});
    auto state = ConfigOptions::Instance().GetString("location.state"s, std::string{});
    auto country = ConfigOptions::Instance().GetString("location.country"s, std::string{});
    // return fmt::format(FMT_STRING("{} {} {}"), city, state, country);
    return boost::join_if(vector{city, state, country}, seperator, [](const auto &str) { return !empty(str); });
}

bool IsWeatherResponseInvalid(const string &response) {
    // An html response is an indicator that the response is invalid.
    return boost::icontains(response, "<title>"s);
}

// "http://wttr.in/?u&format=%l:+%t+%c+%C+%w+%m+%S+%s&lang=en"
// "http://wttr.in/Albuquerque%20NM%20USA?u&format=%l:+%t+%c+%C+%w+%m+%S+%s&lang=en"
// return format(FMT_STRING("/{}?u&format=%l:+%t+%c+%C+%w+%m+%S+%s&lang=en"), location);
optional<string> RequestWeatherData() {
    using namespace mmotd::networking;
    auto location_path = GetLocation(" "s);
    CHECKS(size(location_path) < size_t{256},
           "location city, state, country length ({}) is larger than maximum size",
           size(location_path));
    auto response = HttpRequest::Get(HttpProtocol::HTTPS, WEATHER_SERVICE_URL, location_path, WEATHER_QUERY);
    auto url = HttpRequest::GetUrl(HttpProtocol::HTTPS, WEATHER_SERVICE_URL, location_path, WEATHER_QUERY);
    if (!response) {
        LOG_ERROR("weather response '{}': nullptr", url);
        return nullopt;
    } else if (IsWeatherResponseInvalid(*response)) {
        LOG_ERROR("weather response '{}': html error page", url);
        return nullopt;
    }
    return response;
}

} // namespace

namespace mmotd::information {

static const bool users_logged_in_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::WeatherInfo>(); });

void WeatherInfo::FindInformation() {
    auto weather_data = GetWeatherInfo();
    if (!weather_data) {
        return;
    }

    auto [location_str, weather_str, sunrise_str, sunset_str] = weather_data.value();
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

optional<WeatherInfo::WeatherData> WeatherInfo::GetWeatherInfo() {
    auto weather_response = RequestWeatherData();
    if (!weather_response) {
        return nullopt;
    }

    auto location_str = GetLocation(" "s);
    auto weather_str = *weather_response;
    if (auto i = weather_str.find_first_of(':'); i != string::npos) {
        location_str = boost::trim_copy(weather_str.substr(0, i));
        weather_str = boost::trim_copy(weather_str.substr(i + 1));
    } else {
        LOG_ERROR("weather response appears to be malformed");
        return nullopt;
    }

    const auto sunrise_sunset_regex = regex(R"((\d{2}:\d{2}:\d{2}) (\d{2}:\d{2}:\d{2}))", std::regex::ECMAScript);
    auto match = smatch{};
    if (!regex_search(weather_str, match, sunrise_sunset_regex)) {
        LOG_ERROR("weather information does not seem to include sunrise and sunset");
        return nullopt;
    }

    auto weather = boost::trim_copy(weather_str.substr(0, static_cast<size_t>(match.position(0))));
    LOG_VERBOSE("weather: '{}'", weather);
    auto sunrise_str = weather_str.substr(static_cast<size_t>(match.position(1)), static_cast<size_t>(match.length(1)));
    LOG_VERBOSE("sunrise: '{}'", sunrise_str);
    auto sunset_str = weather_str.substr(static_cast<size_t>(match.position(2)), static_cast<size_t>(match.length(2)));
    LOG_VERBOSE("sunset: '{}'", sunset_str);
    if (empty(weather) || empty(sunrise_str) || empty(sunset_str)) {
        LOG_ERROR("weather data appears to be malformed (weather, sunrise or sunset empty)");
        return nullopt;
    }

    auto sunrise_parsed = ParseTodPrintTod(sunrise_str);
    auto sunset_parsed = ParseTodPrintTod(sunset_str);

    LOG_VERBOSE("weather: '{}, Sunrise (parsed): {}, Sunset (parsed): {}'", weather, *sunrise_parsed, *sunset_parsed);
    return make_tuple(location_str, weather, *sunrise_parsed, *sunset_parsed);
}

} // namespace mmotd::information
