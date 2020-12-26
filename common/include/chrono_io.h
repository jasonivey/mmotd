// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <optional>
#include <string>

#include <fmt/chrono.h>
#include <plog/Log.h>

namespace mmotd::chrono::io {

enum class FromStringFormat {
    TimeFormat, // 18:47:01 or \d{2}:\d{2}\d{2}
};

namespace detail {

struct DateTimeFields {
    int seconds = 0;               /* seconds after the minute [0-60] */
    int minutes = 0;               /* minutes after the hour [0-59] */
    int hours = 0;                 /* hours since midnight [0-23] */
    int month_day = 0;             /* day of the month [1-31] */
    int month = 0;                 /* months since January [0-11] */
    int year = 0;                  /* years since 1900 */
    int week_day = 0;              /* days since Sunday [0-6] */
    int year_day = 0;              /* days since January 1 [0-365] */
    int is_daylight_savings = 0;   /* Daylight Savings Time flag */
    long offset_from_gmt = 0;      /* offset from UTC in seconds */
    const char *timezone_abbr = 0; /* timezone abbreviation */

    enum Field {
        NONE = 0,
        SECONDS = 1,
        MINUTES = 2,
        HOURS = 4,
        MONTH_DAY = 8,
        MONTH = 16,
        YEAR = 32,
        WEEK_DAY = 64,
        YEAR_DAY = 128,
        IS_DAYLIGHT_SAVINGS = 256,
        OFFSET_FROM_GMT = 512,
        TIMEZONE_ABBR = 1024
    };

    Field fields = Field::NONE;

    DateTimeFields() = default;
    DateTimeFields(const tm &date_time_tm, DateTimeFields::Field fields);

    bool IsAnyFieldSet() const { return fields != Field::NONE; }
    bool IsFieldSet(Field field_mask) const { return (fields & field_mask) == field_mask; }

    template<FromStringFormat format>
    static std::optional<DateTimeFields> from_string(std::string) {
        return std::nullopt;
    }

    void UpdateFromTm(const tm &tm_time);
    void UpdateTm(tm &tm_time);
};

inline DateTimeFields::DateTimeFields(const tm &date_time_tm, DateTimeFields::Field set_fields) {
    fields = set_fields;
    UpdateFromTm(date_time_tm);
}

inline void DateTimeFields::UpdateTm(tm &tm_time) {
    if (IsFieldSet(DateTimeFields::Field::SECONDS)) {
        tm_time.tm_sec = seconds;
    }
    if (IsFieldSet(DateTimeFields::Field::MINUTES)) {
        tm_time.tm_min = minutes;
    }
    if (IsFieldSet(DateTimeFields::Field::HOURS)) {
        tm_time.tm_hour = hours;
    }
    if (IsFieldSet(DateTimeFields::Field::MONTH_DAY)) {
        tm_time.tm_mday = month_day;
    }
    if (IsFieldSet(DateTimeFields::Field::MONTH)) {
        tm_time.tm_mon = month;
    }
    if (IsFieldSet(DateTimeFields::Field::YEAR)) {
        tm_time.tm_year = year;
    }
    if (IsFieldSet(DateTimeFields::Field::WEEK_DAY)) {
        tm_time.tm_wday = week_day;
    }
    if (IsFieldSet(DateTimeFields::Field::YEAR_DAY)) {
        tm_time.tm_yday = year_day;
    }
    if (IsFieldSet(DateTimeFields::Field::IS_DAYLIGHT_SAVINGS)) {
        tm_time.tm_isdst = is_daylight_savings;
    }
    if (IsFieldSet(DateTimeFields::Field::OFFSET_FROM_GMT)) {
        tm_time.tm_gmtoff = offset_from_gmt;
    }
    if (IsFieldSet(DateTimeFields::Field::TIMEZONE_ABBR)) {
        tm_time.tm_zone = const_cast<char *>(timezone_abbr);
    }
}

inline void DateTimeFields::UpdateFromTm(const tm &tm_time) {
    if (IsFieldSet(DateTimeFields::Field::SECONDS)) {
        seconds = tm_time.tm_sec;
    }
    if (IsFieldSet(DateTimeFields::Field::MINUTES)) {
        minutes = tm_time.tm_min;
    }
    if (IsFieldSet(DateTimeFields::Field::HOURS)) {
        hours = tm_time.tm_hour;
    }
    if (IsFieldSet(DateTimeFields::Field::MONTH_DAY)) {
        month_day = tm_time.tm_mday;
    }
    if (IsFieldSet(DateTimeFields::Field::MONTH)) {
        month = tm_time.tm_mon;
    }
    if (IsFieldSet(DateTimeFields::Field::YEAR)) {
        year = tm_time.tm_year;
    }
    if (IsFieldSet(DateTimeFields::Field::WEEK_DAY)) {
        week_day = tm_time.tm_wday;
    }
    if (IsFieldSet(DateTimeFields::Field::YEAR_DAY)) {
        year_day = tm_time.tm_yday;
    }
    if (IsFieldSet(DateTimeFields::Field::IS_DAYLIGHT_SAVINGS)) {
        is_daylight_savings = tm_time.tm_isdst;
    }
    if (IsFieldSet(DateTimeFields::Field::OFFSET_FROM_GMT)) {
        offset_from_gmt = tm_time.tm_gmtoff;
    }
    if (IsFieldSet(DateTimeFields::Field::TIMEZONE_ABBR)) {
        timezone_abbr = tm_time.tm_zone;
    }
}

template<>
inline std::optional<DateTimeFields> DateTimeFields::from_string<FromStringFormat::TimeFormat>(std::string input_str) {
    auto date_time_tm = tm{};
    if (strptime(input_str.c_str(), "%H:%M:%S", &date_time_tm) == nullptr) {
        PLOG_ERROR << fmt::format("error while converting string '{}' to a tm time struct", input_str);
        return std::nullopt;
    }
    auto fields = static_cast<DateTimeFields::Field>(DateTimeFields::Field::SECONDS | DateTimeFields::Field::MINUTES |
                                                     DateTimeFields::Field::HOURS);
    return std::make_optional(DateTimeFields(date_time_tm, fields));
}

inline std::optional<DateTimeFields> from_string(std::string input_str, FromStringFormat fmt) {
    if (fmt == FromStringFormat::TimeFormat) {
        return DateTimeFields::from_string<FromStringFormat::TimeFormat>(input_str);
    }
    return std::nullopt;
}

} // namespace detail

inline std::string to_string(std::chrono::system_clock::time_point time_point, std::string chrono_format) {
    auto local_time = fmt::localtime(std::chrono::system_clock::to_time_t(time_point));
    auto time_point_str = fmt::format(chrono_format, local_time);
    auto am_index = time_point_str.rfind("AM");
    auto pm_index = time_point_str.rfind("PM");
    if (am_index != std::string::npos) {
        time_point_str[am_index] = 'a';
        time_point_str[am_index + 1] = 'm';
    } else if (pm_index != std::string::npos) {
        time_point_str[pm_index] = 'p';
        time_point_str[pm_index + 1] = 'm';
    }
    return time_point_str;
}

inline std::optional<std::chrono::system_clock::time_point> from_string(std::string date_time_str,
                                                                        FromStringFormat format_type) {
    using sys_clock = std::chrono::system_clock;

    auto date_time_fields_wrapper = detail::from_string(date_time_str, format_type);
    setenv("TZ", "/usr/share/zoneinfo/MST", 1); // fix_jasoni: POSIX-specific
    if (!date_time_fields_wrapper) {
        PLOG_ERROR << fmt::format("error returned when converting {} to tm structure", date_time_str);
        return std::nullopt;
    }
    auto date_time_fields = *date_time_fields_wrapper;
    if (!date_time_fields.IsAnyFieldSet()) {
        PLOG_ERROR << fmt::format("no fields were set after converting {} to tm structure", date_time_str);
        return std::nullopt;
    }
    auto local_time = fmt::localtime(sys_clock::to_time_t(sys_clock::now()));
    date_time_fields.UpdateTm(local_time);
    auto updated_time_t = std::mktime(&local_time);
    return std::make_optional(sys_clock::from_time_t(updated_time_t));
}

} // namespace mmotd::chrono::io
