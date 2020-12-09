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

namespace mmotd {

static const bool users_logged_in_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::WeatherInfo>(); });

bool WeatherInfo::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        return GetWeatherInfo();
    }
    return has_queried;
}

optional<mmotd::ComputerValues> WeatherInfo::GetInformation() const {
    return !details_.empty() ? make_optional(details_) : nullopt;
}

bool WeatherInfo::GetWeatherInfo() {
    //"http://wttr.in/?u&format=%l:+%t+%c+%C+%w+%m+%S+%s&lang=en"
    //"http://wttr.in/Lehi%20UT%20US?u&format=%l:+%t+%c+%C+%w+%m+%S+%s&lang=en"
    auto http_request = HttpRequest(HttpProtocol::HTTP, "wttr.in");
    auto response = http_request.MakeRequest("/Lehi%20UT%20US?u&format=%l:+%t+%c+%C+%w+%m+%S+%s&lang=en");

    auto match = smatch{};
    const char *regex_str = "(\\d{2}:\\d{2}:\\d{2}) (\\d{2}:\\d{2}:\\d{2})";
    // R"regex(
    // (\d{2}:\d{2}:\d{2}) (\d{2}:\d{2}:\d{2})
    // )regex";
    auto sunrise_sunset_regex = regex(regex_str, std::regex_constants::ECMAScript);
    if (!regex_search(response, match, sunrise_sunset_regex)) {
        PLOG_ERROR << "weather information does not seem to include sunrise and sunset";
        details_.push_back(make_tuple("weather", boost::trim_copy(response)));
        return true;
    }

    auto weather = boost::trim_copy(response.substr(0, match.position(0)));
    PLOG_INFO << format("weather: '{}'", weather);
    auto sunrise = response.substr(match.position(1), match.length(1));
    PLOG_INFO << format("sunrise: '{}'", sunrise);
    auto sunset = response.substr(match.position(2), match.length(2));
    PLOG_INFO << format("sunset: '{}'", sunset);

    auto surise_time_point = mmotd::chrono::io::from_string(sunrise, mmotd::chrono::io::FromStringFormat::TimeFormat);
    auto sunset_time_point = mmotd::chrono::io::from_string(sunset, mmotd::chrono::io::FromStringFormat::TimeFormat);

    if (surise_time_point) {
        auto sunrise_str = mmotd::chrono::io::to_string(*surise_time_point, "{:%I:%M:%S%p}");
        weather += format(", {}", sunrise_str);
    } else {
        weather += format(", {}", sunrise);
    }
    if (sunset_time_point) {
        auto sunset_str = mmotd::chrono::io::to_string(*sunset_time_point, "{:%I:%M:%S%p}");
        weather += format(", {}", sunset_str);
    } else {
        weather += format(", {}", sunset);
    }
    PLOG_INFO << format("weather: '{}'", weather);
    details_.push_back(make_tuple("weather", weather));
    return true;
}

} // namespace mmotd
