// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <cstdint>
#include <iterator>
#include <string>

namespace mmotd::platform {

class Temperature {
public:
    enum Units { Celsius, Fahrenheit };

    explicit Temperature(double value, Units units = Units::Celsius);
    Temperature() = default;
    ~Temperature() = default;
    Temperature(Temperature &) = default;
    Temperature(Temperature &&) = default;
    Temperature &operator=(const Temperature &) = default;
    Temperature &operator=(Temperature &&) = default;

    bool empty() const noexcept;
    std::string to_string() const;

    double GetTemperature(Units units = Units::Celsius) const noexcept;
    std::string GetTemperatureIcon() const noexcept;

private:
    enum class Scale { Unknown, Low, Medium, High };

    Scale GetScale() const noexcept;

    double value_ = 0.0;
};

Temperature GetCpuTemperature();
Temperature GetGpuTemperature();

} // namespace mmotd::platform
