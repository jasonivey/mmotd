// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/platform/hardware_temperature.h"

#include <cmath>
#include <string>

#include <fmt/format.h>

using namespace std;

namespace {

inline double ConvertToCelcius(double value) {
    return (value - 32.0) * (5.0 / 9.0);
}

inline double ConvertToFahrenheit(double value) {
    return (value * (9.0 / 5.0)) + 32.0;
}

} // namespace

namespace mmotd::platform {

Temperature::Temperature(double value, Units units) :
    value_(units == Units::Fahrenheit ? ConvertToCelcius(value) : value) {}

bool Temperature::empty() const noexcept {
    return !value_;
}

string Temperature::to_string() const {
    return fmt::format("{} {:.1f}°C {:.1f}°F",
                       GetTemperatureIcon(),
                       GetTemperature(Units::Celsius),
                       GetTemperature(Units::Fahrenheit));
}

double Temperature::GetTemperature(Units units) const noexcept {
    if (!value_) {
        return 0.0;
    } else if (units == Units::Fahrenheit) {
        return ConvertToFahrenheit(*value_);
    } else {
        return *value_;
    }
}

string Temperature::GetTemperatureIcon() const noexcept {
    static const char *THERMOMETER_HALF = "\uE20A";
    static const char *THERMOMETER_FULL = "\uE20B";
    static const char *THERMOMETER_EMPTY = "\uE20C";
    static const char *FIRE_UNKNOWN = "\uF490";
    switch (GetScale()) {
        case Scale::Low:
            return THERMOMETER_EMPTY;
            break;
        case Scale::Medium:
            return THERMOMETER_HALF;
            break;
        case Scale::High:
            return THERMOMETER_FULL;
            break;
        case Scale::Unknown:
        default:
            return FIRE_UNKNOWN;
            break;
    }
}

Temperature::Scale Temperature::GetScale() const noexcept {
    static constexpr auto MEDIUM_TEMPERATURE = 75.0;
    static constexpr auto HIGH_TEMPERATURE = 90.0;
    if (value_ < MEDIUM_TEMPERATURE) {
        return Scale::Low;
    } else if (value_ < HIGH_TEMPERATURE) {
        return Scale::Medium;
    } else {
        return Scale::High;
    }
}

} // namespace mmotd::platform
