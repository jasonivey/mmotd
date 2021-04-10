// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <optional>
#include <string>

#include <fmt/chrono.h>
#include <plog/Log.h>

#include <time.h>

namespace mmotd::chrono::io {

enum class FromStringFormat {
    TimeFormat, // 18:47:01 or \d{2}:\d{2}\d{2}
};

namespace detail {

class DateTimeFields {
public:
    enum Field : uint32_t {
        NONE = 1 << 0,
        SECONDS = 1 << 1,
        MINUTES = 1 << 2,
        HOURS = 1 << 3,
        MONTH_DAY = 1 << 4,
        MONTH = 1 << 5,
        YEAR = 1 << 6,
        WEEK_DAY = 1 << 7,
        YEAR_DAY = 1 << 8,
        IS_DAYLIGHT_SAVINGS = 1 << 9,
        OFFSET_FROM_GMT = 1 << 10,
        TIMEZONE_ABBR = 1 << 11
    };

    friend constexpr Field operator|(Field a, Field b) noexcept {
        return static_cast<Field>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }
    friend constexpr Field operator&(Field a, Field b) noexcept {
        return static_cast<Field>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }

    DateTimeFields() = default;
    DateTimeFields(const std::tm &date_time_tm, DateTimeFields::Field fields);

    bool IsAnyFieldSet() const noexcept { return fields_ != Field::NONE; }
    bool IsFieldSet(Field field_mask) const noexcept { return (fields_ & field_mask) == field_mask; }

    template<FromStringFormat format>
    static std::optional<DateTimeFields> from_string(std::string) {
        return std::nullopt;
    }

    void Import(const std::tm &tm_time) noexcept;
    std::tm Export(const std::tm &tm_time) noexcept;

private:
    std::tm date_time_tm_{};
    Field fields_ = Field::NONE;
};

inline DateTimeFields::DateTimeFields(const std::tm &date_time_tm, DateTimeFields::Field fields) {
    fields_ = fields;
    Import(date_time_tm);
}

inline void DateTimeFields::Import(const std::tm &tm_time) noexcept {
    if (IsFieldSet(DateTimeFields::Field::SECONDS)) {
        date_time_tm_.tm_sec = tm_time.tm_sec;
    }
    if (IsFieldSet(DateTimeFields::Field::MINUTES)) {
        date_time_tm_.tm_min = tm_time.tm_min;
    }
    if (IsFieldSet(DateTimeFields::Field::HOURS)) {
        date_time_tm_.tm_hour = tm_time.tm_hour;
    }
    if (IsFieldSet(DateTimeFields::Field::MONTH_DAY)) {
        date_time_tm_.tm_mday = tm_time.tm_mday;
    }
    if (IsFieldSet(DateTimeFields::Field::MONTH)) {
        date_time_tm_.tm_mon = tm_time.tm_mon;
    }
    if (IsFieldSet(DateTimeFields::Field::YEAR)) {
        date_time_tm_.tm_year = tm_time.tm_year;
    }
    if (IsFieldSet(DateTimeFields::Field::WEEK_DAY)) {
        date_time_tm_.tm_wday = tm_time.tm_wday;
    }
    if (IsFieldSet(DateTimeFields::Field::YEAR_DAY)) {
        date_time_tm_.tm_yday = tm_time.tm_yday;
    }
    if (IsFieldSet(DateTimeFields::Field::IS_DAYLIGHT_SAVINGS)) {
        date_time_tm_.tm_isdst = tm_time.tm_isdst;
    }
    if (IsFieldSet(DateTimeFields::Field::OFFSET_FROM_GMT)) {
        date_time_tm_.tm_gmtoff = tm_time.tm_gmtoff;
    }
    if (IsFieldSet(DateTimeFields::Field::TIMEZONE_ABBR)) {
        date_time_tm_.tm_zone = tm_time.tm_zone;
    }
}

inline std::tm DateTimeFields::Export(const std::tm &tm_time) noexcept {
    auto new_tm{tm_time};
    if (IsFieldSet(DateTimeFields::Field::SECONDS)) {
        new_tm.tm_sec = date_time_tm_.tm_sec;
    }
    if (IsFieldSet(DateTimeFields::Field::MINUTES)) {
        new_tm.tm_min = date_time_tm_.tm_min;
    }
    if (IsFieldSet(DateTimeFields::Field::HOURS)) {
        new_tm.tm_hour = date_time_tm_.tm_hour;
    }
    if (IsFieldSet(DateTimeFields::Field::MONTH_DAY)) {
        new_tm.tm_mday = date_time_tm_.tm_mday;
    }
    if (IsFieldSet(DateTimeFields::Field::MONTH)) {
        new_tm.tm_mon = date_time_tm_.tm_mon;
    }
    if (IsFieldSet(DateTimeFields::Field::YEAR)) {
        new_tm.tm_year = date_time_tm_.tm_year;
    }
    if (IsFieldSet(DateTimeFields::Field::WEEK_DAY)) {
        new_tm.tm_wday = date_time_tm_.tm_wday;
    }
    if (IsFieldSet(DateTimeFields::Field::YEAR_DAY)) {
        new_tm.tm_yday = date_time_tm_.tm_yday;
    }
    if (IsFieldSet(DateTimeFields::Field::IS_DAYLIGHT_SAVINGS)) {
        new_tm.tm_isdst = date_time_tm_.tm_isdst;
    }
    if (IsFieldSet(DateTimeFields::Field::OFFSET_FROM_GMT)) {
        new_tm.tm_gmtoff = date_time_tm_.tm_gmtoff;
    }
    if (IsFieldSet(DateTimeFields::Field::TIMEZONE_ABBR)) {
        new_tm.tm_zone = const_cast<char *>(date_time_tm_.tm_zone);
    }
    return new_tm;
}

template<>
inline std::optional<DateTimeFields> DateTimeFields::from_string<FromStringFormat::TimeFormat>(std::string input_str) {
    auto date_time_tm = std::tm{};
    if (strptime(input_str.c_str(), "%H:%M:%S", &date_time_tm) == nullptr) {
        PLOG_ERROR << fmt::format("error while converting string '{}' to a std::tm time struct", input_str);
        return std::nullopt;
    }
    auto fields = DateTimeFields::Field::SECONDS | DateTimeFields::Field::MINUTES | DateTimeFields::Field::HOURS;
    return std::make_optional(DateTimeFields(date_time_tm, fields));
}

inline std::optional<DateTimeFields> from_string_impl(std::string input_str, FromStringFormat fmt) {
    if (fmt == FromStringFormat::TimeFormat) {
        return DateTimeFields::from_string<FromStringFormat::TimeFormat>(input_str);
    }
    return std::nullopt;
}

inline std::string to_string(std::chrono::system_clock::time_point time_point, std::string chrono_format) {
    static auto initialized_time_zone = false;
    if (!initialized_time_zone) {
        initialized_time_zone = true;
        setenv("TZ", "/usr/share/zoneinfo/MST", 1); // fix_jasoni: POSIX-specific
        tzset();
    }
    auto local_time = fmt::localtime(std::chrono::system_clock::to_time_t(time_point));
    PLOG_VERBOSE << fmt::format("setting tm_isdst: {}",
                                (daylight == 0 ? "(0) false" : (daylight == 1 ? "(1) true" : "-1 undetermined")));
    local_time.tm_isdst = daylight;
    auto time_point_str = fmt::format(chrono_format, local_time);
    if (auto am_index = time_point_str.rfind("AM"); am_index != std::string::npos) {
        time_point_str[am_index] = 'a';
        time_point_str[am_index + 1] = 'm';
    }
    if (auto pm_index = time_point_str.rfind("PM"); pm_index != std::string::npos) {
        time_point_str[pm_index] = 'p';
        time_point_str[pm_index + 1] = 'm';
    }
    return time_point_str;
}

inline std::optional<std::chrono::system_clock::time_point> from_string(std::string input_str,
                                                                        FromStringFormat format_type) {
    using sys_clock = std::chrono::system_clock;
    auto date_time_fields_holder = from_string_impl(input_str, format_type);
    setenv("TZ", "/usr/share/zoneinfo/MST", 1); // fix_jasoni: POSIX-specific
    if (!date_time_fields_holder) {
        PLOG_ERROR << fmt::format("error returned when converting {} to std::tm structure", input_str);
        return std::nullopt;
    }
    auto date_time_fields = *date_time_fields_holder;
    if (!date_time_fields.IsAnyFieldSet()) {
        PLOG_ERROR << fmt::format("no fields were set after converting {} to std::tm structure", input_str);
        return std::nullopt;
    }
    auto now_tm = fmt::localtime(sys_clock::to_time_t(sys_clock::now()));
    auto input_tm = date_time_fields.Export(now_tm);
    auto input_time_since_epoch = std::mktime(&input_tm);
    return std::make_optional(sys_clock::from_time_t(input_time_since_epoch));
}

inline int get_current_hour() {
    using sys_clock = std::chrono::system_clock;
    setenv("TZ", "/usr/share/zoneinfo/MST", 1); // fix_jasoni: POSIX-specific
    time_t now_time_since_epoch = sys_clock::to_time_t(sys_clock::now());
    auto now_tm = fmt::localtime(now_time_since_epoch);
    return now_tm.tm_hour;
}

} // namespace detail

using detail::from_string;
using detail::get_current_hour;
using detail::to_string;

} // namespace mmotd::chrono::io
