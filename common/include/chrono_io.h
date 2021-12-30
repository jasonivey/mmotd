// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/config_options.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <locale>
#include <optional>
#include <sstream>
#include <string>

#include <boost/algorithm/string/replace.hpp>
#include <date/date.h>
#include <date/tz.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace mmotd::chrono::io {

namespace detail {

inline std::string GetTimeZoneStr() {
    using namespace mmotd::core;
    static auto time_zone_str = std::string{};
    if (std::empty(time_zone_str)) {
        time_zone_str = ConfigOptions::Instance().GetString("location.timezone", std::string{});
    }
    return time_zone_str;
}

inline const date::time_zone *GetTimeZone() {
    static const date::time_zone *tz = nullptr;
    if (tz == nullptr) {
        static const auto time_zone_str = GetTimeZoneStr();
        if (!std::empty(time_zone_str)) {
            tz = date::locate_zone(time_zone_str);
        }
    }
    if (tz == nullptr) {
        tz = date::current_zone();
    }
    return tz;
}

inline std::string lower_case_merīdiem(std::string input) {
    using boost::algorithm::replace_all_copy;
    static constexpr const std::string_view ANTE_MERIDIEM = "AM";
    static constexpr const std::string_view ANTE_MERIDIEM_LOWER = "am";
    static constexpr const std::string_view POST_MERIDIEM = "PM";
    static constexpr const std::string_view POST_MERIDIEM_LOWER = "pm";
    return replace_all_copy(replace_all_copy(input, ANTE_MERIDIEM, ANTE_MERIDIEM_LOWER),
                            POST_MERIDIEM,
                            POST_MERIDIEM_LOWER);
}

inline std::string to_string(date::time_of_day<std::chrono::seconds> tod, const char *chrono_format) {
    auto fields = date::fields{tod};
    auto result = date::format(chrono_format, fields);
    return lower_case_merīdiem(result);
}

template<class Clock, class Duration>
inline std::string to_string(std::chrono::time_point<Clock, Duration> time_point, const char *chrono_format) {
    auto time_point_zoned = date::make_zoned(GetTimeZone(), time_point);
    auto result = date::format(chrono_format, time_point_zoned);
    return lower_case_merīdiem(result);
}

inline auto from_string(std::string input, const char *chrono_format) {
    using namespace std::chrono;
    auto seconds_since_midnight = std::chrono::seconds{};
    std::istringstream stream{input};
    stream >> date::parse(chrono_format, seconds_since_midnight);
    auto tod = date::make_time(seconds_since_midnight);
    return !stream.fail() ? std::make_optional(tod) : std::nullopt;
}

inline std::optional<std::size_t> get_current_hour() {
    auto now_time_point = date::make_zoned(GetTimeZone(), std::chrono::system_clock::now());
    auto hour_str = date::format("%H", now_time_point);
    const auto &loc = std::locale();
    if (size(hour_str) != std::size_t{2}) {
        return std::nullopt;
    } else if (!std::isdigit(hour_str.front(), loc) || !std::isdigit(hour_str.back(), loc)) {
        return std::nullopt;
    } else {
        if (hour_str.front() == '0') {
            hour_str = hour_str.substr(1);
        }
        auto hour = stoull(hour_str, nullptr, 10);
        if (hour > 23) {
            hour = std::size_t{23};
        }
        return {hour};
    }
}

} // namespace detail

using detail::from_string;
using detail::get_current_hour;
using detail::GetTimeZone;
using detail::to_string;

} // namespace mmotd::chrono::io
