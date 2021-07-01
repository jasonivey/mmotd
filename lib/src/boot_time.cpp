// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/chrono_io.h"
#include "common/include/information.h"
#include "common/include/information_definitions.h"
#include "common/include/logging.h"
#include "common/include/posix_error.h"
#include "lib/include/boot_time.h"
#include "lib/include/computer_information.h"
#include "lib/include/platform/boot_time.h"

#include <chrono>
#include <ctime>
#include <iterator>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <date/date.h>
#include <fmt/format.h>

using fmt::format;
using namespace std;

bool gLinkBootTime = false;

namespace mmotd::information {

static const bool boot_time_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::BootTime>(); });

template<typename T>
string AddDurationToString(string current, T count, string name) {
    if (count < 1) {
        return current;
    }
    auto plural = count > 1 ? string{"s"} : string{};
    if (!empty(current)) {
        current += ", ";
    }
    return current + format(FMT_STRING("{} {}{}"), count, name, plural);
}

void BootTime::FindInformation() {
    auto boot_time_holder = mmotd::platform::GetBootTime();
    if (!boot_time_holder) {
        LOG_ERROR("unable to find boot time property");
    }

    const auto &boot_time = *boot_time_holder;
    const auto now_time = std::chrono::system_clock::now();
    auto boot_time_duration = std::chrono::duration_cast<std::chrono::seconds>(now_time - boot_time);

    auto years = std::chrono::duration_cast<std::chrono::years>(boot_time_duration);
    auto boot_time_str = AddDurationToString(string{}, years.count(), "year");
    boot_time_duration -= years;

    auto months = std::chrono::duration_cast<std::chrono::months>(boot_time_duration);
    boot_time_str = AddDurationToString(boot_time_str, months.count(), "month");
    boot_time_duration -= months;

    auto weeks = std::chrono::duration_cast<std::chrono::weeks>(boot_time_duration);
    boot_time_str = AddDurationToString(boot_time_str, weeks.count(), "week");
    boot_time_duration -= weeks;

    auto days = std::chrono::duration_cast<std::chrono::days>(boot_time_duration);
    boot_time_str = AddDurationToString(boot_time_str, days.count(), "day");
    boot_time_duration -= days;

    auto hours = std::chrono::duration_cast<std::chrono::hours>(boot_time_duration);
    boot_time_str = AddDurationToString(boot_time_str, hours.count(), "hour");
    boot_time_duration -= hours;

    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(boot_time_duration);
    boot_time_str = AddDurationToString(boot_time_str, minutes.count(), "minute");
    boot_time_duration -= minutes;

    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(boot_time_duration);
    boot_time_str = AddDurationToString(boot_time_str, seconds.count(), "second");

    auto up_time_info = GetInfoTemplate(InformationId::ID_BOOT_TIME_UP_TIME);
    up_time_info.SetValue(boot_time_str);
    AddInformation(up_time_info);

    auto boot_time_info = GetInfoTemplate(InformationId::ID_BOOT_TIME_BOOT_TIME);
    boot_time_info.SetValue(chrono::io::to_string(boot_time, "%a, %d-%h-%Y %I:%M%p %Z"));
    AddInformation(boot_time_info);
}

} // namespace mmotd::information
