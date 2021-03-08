// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/chrono_io.h"
#include "lib/include/computer_information.h"
#include "lib/include/http_request.h"
#include "lib/include/weather_info.h"

#include <iterator>
#include <regex>
#include <string>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkWeatherInfo = false;

namespace mmotd::information {

static constexpr const char *LOCATION = "Lehi UT US";

static const bool users_logged_in_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::WeatherInfo>(); });

bool WeatherInfo::FindInformation() {
    auto [location_str, weather_str, sunrise_str, sunset_str] = GetWeatherInfo();
    if (empty(weather_str)) {
        return false;
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
        return true;
    }

    auto location = GetInfoTemplate(InformationId::ID_WEATHER_LOCATION);
    location.SetValueArgs(location_str);
    AddInformation(location);

    return true;
}

tuple<string, string, string, string> WeatherInfo::GetWeatherInfo() {
    using boost::trim_copy, boost::replace_all_copy;
    using namespace mmotd::chrono::io;
    using namespace mmotd::networking;

    //"http://wttr.in/?u&format=%l:+%t+%c+%C+%w+%m+%S+%s&lang=en"
    //"http://wttr.in/Lehi%20UT%20US?u&format=%l:+%t+%c+%C+%w+%m+%S+%s&lang=en"
    auto http_request = HttpRequest(HttpProtocol::HTTP, "wttr.in");
    auto url = format("/{}?u&format=%l:+%t+%c+%C+%w+%m+%S+%s&lang=en", replace_all_copy(string{LOCATION}, " ", "%20"));
    auto http_response = http_request.MakeRequest(url);
    if (!http_response) {
        return make_tuple(string{}, string{}, string{}, string{});
    }

    auto location_str = string{LOCATION};
    auto weather_str = *http_response;
    if (auto i = weather_str.find_first_of(':'); i != string::npos) {
        location_str = boost::trim_copy(weather_str.substr(0, i));
        weather_str = boost::trim_copy(weather_str.substr(i + 1));
    }

    const auto sunrise_sunset_regex = regex(R"((\d{2}:\d{2}:\d{2}) (\d{2}:\d{2}:\d{2}))", std::regex::ECMAScript);
    auto match = smatch{};
    if (!regex_search(weather_str, match, sunrise_sunset_regex)) {
        PLOG_ERROR << "weather information does not seem to include sunrise and sunset";
        return make_tuple(location_str, weather_str, string{}, string{});
    }

    auto weather = trim_copy(weather_str.substr(0, match.position(0)));
    PLOG_INFO << format("weather: '{}'", weather);
    auto sunrise_str = weather_str.substr(match.position(1), match.length(1));
    PLOG_INFO << format("sunrise: '{}'", sunrise_str);
    auto sunset_str = weather_str.substr(match.position(2), match.length(2));
    PLOG_INFO << format("sunset: '{}'", sunset_str);

    auto surise_time_point = from_string(sunrise_str, FromStringFormat::TimeFormat);
    auto sunset_time_point = from_string(sunset_str, FromStringFormat::TimeFormat);

    if (surise_time_point) {
        sunrise_str = to_string(*surise_time_point, "{:%I:%M:%S%p}");
    }
    if (sunset_time_point) {
        sunset_str = to_string(*sunset_time_point, "{:%I:%M:%S%p}");
    }
    PLOG_INFO << format("weather: '{}, Sunrise: {}, Sunset: {}'", weather, sunrise_str, sunset_str);
    return make_tuple(location_str, weather, sunrise_str, sunset_str);
}

} // namespace mmotd::information
