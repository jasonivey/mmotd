#include "common/include/chrono-io.h"

#include <chrono>
#include <fmt/chrono.h>

using namespace std;
using fmt::format;

namespace mmotd { namespace chrono { namespace io {

namespace detail {
#if 0
template<typename Clock, typename Duration>
std::tm to_calendar_time(std::chrono::time_point<Clock, Duration> time_point) {
    using namespace date;
    auto date = floor<days>(time_point);
    auto ymd = year_month_day(date);
    //auto weekday = year_month_weekday(date).weekday_indexed().weekday();
    auto tod = make_time(time_point - date);
    //days daysSinceJan1 = date - sys_days(ymd.year() / 1 / 1);

    std::tm result;
    std::memset(&result, 0, sizeof(result));
    result.tm_sec = tod.seconds().count();
    result.tm_min = tod.minutes().count();
    result.tm_hour = tod.hours().count();
    result.tm_mday = unsigned(ymd.day());
    result.tm_mon = unsigned(ymd.month()) - 1u; // Zero-based!
    result.tm_year = int(ymd.year()) - 1900;
    //result.tm_wday = weekday.c_encoding();
    //result.tm_yday = daysSinceJan1.count();
    //result.tm_isdst = -1; // Information not available
    return result;
}
#endif
} // namespace detail

string to_string(std::chrono::system_clock::time_point /*time_point*/, string_view /*chrono_format*/) {
    //auto t = detail::to_calendar_time(time_point);
    //return format(chrono_format.data(), t);
    return string{};
}

}}} // namespace mmotd::chrono::io
