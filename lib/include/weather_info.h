// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "lib/include/information_provider.h"

#include <cstdint>
#include <optional>
#include <string>

namespace mmotd {

class WeatherInfo : public InformationProvider {
public:
    WeatherInfo() = default;

    std::string GetName() const override { return std::string{"weather info"}; }
    bool QueryInformation() override;
    std::optional<mmotd::ComputerValues> GetInformation() const override;

private:
    bool GetWeatherInfo();

    mmotd::ComputerValues details_;
};

} // namespace mmotd
