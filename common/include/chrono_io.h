// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/assertion/include/assertion.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <locale>
#include <optional>
#include <sstream>
#include <string>

#include <date/date.h>
#include <date/tz.h>
#include <fmt/ostream.h>
#include <plog/Log.h>

namespace mmotd::chrono::io {

namespace detail {

inline std::string to_string(date::time_of_day<std::chrono::seconds> tod, std::string chrono_format) {
    auto fields = date::fields{tod};
    auto result = date::format(chrono_format, fields);
    if (auto am_index = result.rfind("AM"); am_index != std::string::npos) {
        result[am_index] = 'a';
        result[am_index + 1] = 'm';
    }
    if (auto pm_index = result.rfind("PM"); pm_index != std::string::npos) {
        result[pm_index] = 'p';
        result[pm_index + 1] = 'm';
    }
    return result;
}

template<class Clock, class Duration>
inline std::string to_string(std::chrono::time_point<Clock, Duration> time_point, std::string chrono_format) {
    auto time_point_zoned = date::make_zoned(date::locate_zone("America/Denver"), time_point);
    // auto time_point_zoned = date::make_zoned(date::locate_zone("Americas/Denver"), time_point);
    auto result = date::format(chrono_format, time_point_zoned);
    if (auto am_index = result.rfind("AM"); am_index != std::string::npos) {
        result[am_index] = 'a';
        result[am_index + 1] = 'm';
    }
    if (auto pm_index = result.rfind("PM"); pm_index != std::string::npos) {
        result[pm_index] = 'p';
        result[pm_index + 1] = 'm';
    }
    return result;
}

inline auto from_string(std::string input, std::string chrono_format) {
    using namespace std::chrono;
    auto seconds_since_midnight = std::chrono::seconds{};
    std::istringstream stream{input};
    stream >> date::parse(chrono_format, seconds_since_midnight);
    auto tod = date::make_time(seconds_since_midnight);
    PLOG_VERBOSE << fmt::format(FMT_STRING("parsed time: {}, using format: {}, from string: '{}'"),
                                tod,
                                chrono_format,
                                input);
    if (stream.fail()) {
        PLOG_ERROR << fmt::format(FMT_STRING("failed to parse string: '{}', using format: {}"), input, chrono_format);
    }
    return !stream.fail() ? std::make_optional(tod) : std::nullopt;
}

inline std::optional<std::size_t> get_current_hour() {
    auto now_time_point = date::make_zoned(date::locate_zone("America/Denver"), std::chrono::system_clock::now());
    auto hour_str = date::format("%H", now_time_point);
    const auto &loc = std::locale();
    if (size(hour_str) != std::size_t{2}) {
        PLOG_ERROR << fmt::format(FMT_STRING("hour is not two characters wide: '{}', current time: '{}'"),
                                  hour_str,
                                  now_time_point);
        return std::nullopt;
    } else if (!std::isdigit(hour_str.front(), loc) || !std::isdigit(hour_str.back(), loc)) {
        PLOG_ERROR << fmt::format(FMT_STRING("hour is not digit characters: '{}', current time: '{}'"),
                                  hour_str,
                                  now_time_point);
        return std::nullopt;
    } else {
        if (hour_str.front() == '0') {
            hour_str = hour_str.substr(1);
        }
        auto hour = stoull(hour_str, nullptr, 10);
        PLOG_VERBOSE << fmt::format(FMT_STRING("current hour: '{}', current time: '{}'"), hour, now_time_point);
        if (hour > 23) {
            PLOG_VERBOSE << fmt::format(
                FMT_STRING("current hour is outside normal range {{0-24]: '{}', new hour: {}, current time: '{}'"),
                hour,
                std::size_t{23},
                now_time_point);
            hour = std::size_t{23};
        }
        return {hour};
    }
}

} // namespace detail

using detail::from_string;
using detail::get_current_hour;
using detail::to_string;

} // namespace mmotd::chrono::io
