#include "common/include/chrono_io.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <optional>

#include <fmt/chrono.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

namespace mmotd::chrono::io {

namespace detail {

struct DateTimeFields {
    int seconds = 0;             /* seconds after the minute [0-60] */
    int minutes = 0;             /* minutes after the hour [0-59] */
    int hours = 0;               /* hours since midnight [0-23] */
    int month_day = 0;           /* day of the month [1-31] */
    int month = 0;               /* months since January [0-11] */
    int year = 0;                /* years since 1900 */
    int week_day = 0;            /* days since Sunday [0-6] */
    int year_day = 0;            /* days since January 1 [0-365] */
    int is_daylight_savings = 0; /* Daylight Savings Time flag */
    long offset_from_gmt = 0;    /* offset from UTC in seconds */
    char *timezone_abbr = 0;     /* timezone abbreviation */

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
    static optional<DateTimeFields> from_string(string) {}

    void UpdateFromTm(const tm &tm_time);
    void UpdateTm(tm &tm_time);
};

DateTimeFields::DateTimeFields(const tm &date_time_tm, DateTimeFields::Field set_fields) {
    fields = set_fields;
    UpdateFromTm(date_time_tm);
}

void DateTimeFields::UpdateTm(tm &tm_time) {
    if (IsFieldSet(detail::DateTimeFields::Field::SECONDS)) {
        PLOG_INFO << format("updating seconds from: {}, to: {}", tm_time.tm_sec, seconds);
        tm_time.tm_sec = seconds;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::MINUTES)) {
        PLOG_INFO << format("updating minutes from: {}, to: {}", tm_time.tm_min, minutes);
        tm_time.tm_min = minutes;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::HOURS)) {
        PLOG_INFO << format("updating hours from: {}, to: {}", tm_time.tm_hour, hours);
        tm_time.tm_hour = hours;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::MONTH_DAY)) {
        PLOG_INFO << format("updating month day from: {}, to: {}", tm_time.tm_mday, month_day);
        tm_time.tm_mday = month_day;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::MONTH)) {
        PLOG_INFO << format("updating month from: {}, to: {}", tm_time.tm_mon, month);
        tm_time.tm_mon = month;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::YEAR)) {
        PLOG_INFO << format("updating year from: {}, to: {}", tm_time.tm_year, year);
        tm_time.tm_year = year;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::WEEK_DAY)) {
        PLOG_INFO << format("updating week day from: {}, to: {}", tm_time.tm_wday, week_day);
        tm_time.tm_wday = week_day;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::YEAR_DAY)) {
        PLOG_INFO << format("updating year day from: {}, to: {}", tm_time.tm_yday, year_day);
        tm_time.tm_yday = year_day;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::IS_DAYLIGHT_SAVINGS)) {
        PLOG_INFO << format("updating is daylight savings from: {}, to: {}", tm_time.tm_isdst, is_daylight_savings);
        tm_time.tm_isdst = is_daylight_savings;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::OFFSET_FROM_GMT)) {
        PLOG_INFO << format("updating offset from gmt from: {}, to: {}", tm_time.tm_gmtoff, offset_from_gmt);
        tm_time.tm_gmtoff = offset_from_gmt;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::TIMEZONE_ABBR)) {
        PLOG_INFO << format("updating timezone abbr from: {}, to: {}", tm_time.tm_zone, timezone_abbr);
        tm_time.tm_zone = timezone_abbr;
    }
}

void DateTimeFields::UpdateFromTm(const tm &tm_time) {
    if (IsFieldSet(detail::DateTimeFields::Field::SECONDS)) {
        PLOG_INFO << format("updating seconds from: {}, to: {}", seconds, tm_time.tm_sec);
        seconds = tm_time.tm_sec;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::MINUTES)) {
        PLOG_INFO << format("updating minutes from: {}, to: {}", minutes, tm_time.tm_min);
        minutes = tm_time.tm_min;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::HOURS)) {
        PLOG_INFO << format("updating hours from: {}, to: {}", hours, tm_time.tm_hour);
        hours = tm_time.tm_hour;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::MONTH_DAY)) {
        PLOG_INFO << format("updating month day from: {}, to: {}", month_day, tm_time.tm_mday);
        month_day = tm_time.tm_mday;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::MONTH)) {
        PLOG_INFO << format("updating month from: {}, to: {}", month, tm_time.tm_mon);
        month = tm_time.tm_mon;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::YEAR)) {
        PLOG_INFO << format("updating year from: {}, to: {}", year, tm_time.tm_year);
        year = tm_time.tm_year;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::WEEK_DAY)) {
        PLOG_INFO << format("updating week day from: {}, to: {}", week_day, tm_time.tm_wday);
        week_day = tm_time.tm_wday;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::YEAR_DAY)) {
        PLOG_INFO << format("updating year day from: {}, to: {}", year_day, tm_time.tm_yday);
        year_day = tm_time.tm_yday;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::IS_DAYLIGHT_SAVINGS)) {
        PLOG_INFO << format("updating is daylight savings from: {}, to: {}", is_daylight_savings, tm_time.tm_isdst);
        is_daylight_savings = tm_time.tm_isdst;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::OFFSET_FROM_GMT)) {
        PLOG_INFO << format("updating offset from gmt from: {}, to: {}", offset_from_gmt, tm_time.tm_gmtoff);
        offset_from_gmt = tm_time.tm_gmtoff;
    }
    if (IsFieldSet(detail::DateTimeFields::Field::TIMEZONE_ABBR)) {
        PLOG_INFO << format("updating timezone abbr from: {}, to: {}", timezone_abbr, tm_time.tm_zone);
        timezone_abbr = tm_time.tm_zone;
    }
}

template<>
optional<DateTimeFields> DateTimeFields::from_string<FromStringFormat::TimeFormat>(string input_str) {
    auto date_time_tm = tm{};
    if (strptime(input_str.c_str(), "%H:%M:%S", &date_time_tm) == nullptr) {
        PLOG_ERROR << format("error while converting string '{}' to a tm time struct", input_str);
        return nullopt;
    }
    auto fields = static_cast<DateTimeFields::Field>(DateTimeFields::Field::SECONDS | DateTimeFields::Field::MINUTES |
                                                     DateTimeFields::Field::HOURS);
    return make_optional(DateTimeFields(date_time_tm, fields));
}

optional<DateTimeFields> from_string(string input_str, FromStringFormat fmt) {
    if (fmt == FromStringFormat::TimeFormat) {
        return DateTimeFields::from_string<FromStringFormat::TimeFormat>(input_str);
    }
    return nullopt;
}

} // namespace detail

string to_string(std::chrono::system_clock::time_point time_point, std::string chrono_format) {
    auto local_time = fmt::localtime(std::chrono::system_clock::to_time_t(time_point));
    auto time_point_str = fmt::format(chrono_format, local_time);
    auto am_index = time_point_str.rfind("AM");
    auto pm_index = time_point_str.rfind("PM");
    if (am_index != string::npos) {
        time_point_str[am_index] = 'a';
        time_point_str[am_index + 1] = 'm';
    } else if (pm_index != string::npos) {
        time_point_str[pm_index] = 'p';
        time_point_str[pm_index + 1] = 'm';
    }
    return time_point_str;
}

optional<std::chrono::system_clock::time_point> from_string(std::string date_time_str, FromStringFormat format_type) {
    auto date_time_fields_wrapper = detail::from_string(date_time_str, format_type);
    setenv("TZ", "/usr/share/zoneinfo/MST", 1); // fix_jasoni: POSIX-specific
    if (!date_time_fields_wrapper) {
        PLOG_ERROR << format("error returned when converting {} to tm structure", date_time_str);
        return nullopt;
    }
    auto date_time_fields = *date_time_fields_wrapper;
    if (!date_time_fields.IsAnyFieldSet()) {
        PLOG_ERROR << format("no fields were set after converting {} to tm structure", date_time_str);
        return nullopt;
    }
    auto now_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    PLOG_INFO << format("now time_t value: {}", now_time_t);
    auto local_time = fmt::localtime(now_time_t);
    date_time_fields.UpdateTm(local_time);
    auto updated_time_t = mktime(&local_time);
    PLOG_INFO << format("modified time_t value: {}", updated_time_t);
    return make_optional(std::chrono::system_clock::from_time_t(updated_time_t));
}

} // namespace mmotd::chrono::io
