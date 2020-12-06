// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/http_request.h"
#include "lib/include/weather_info.h"

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
    auto http_request = HttpRequest(HttpProtocol::HTTP, "wttr.in");
    auto response = http_request.MakeRequest("/?u&format=%l:+%t+%c+%C+%w+%m+%S+%s&lang=en");
    PLOG_INFO << "weather http response: " << response;
    return true;
}

} // namespace mmotd
