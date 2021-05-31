// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"
#include "lib/include/information_provider.h"

#include <cstdint>
#include <string>
#include <tuple>

namespace mmotd::information {

class WeatherInfo : public InformationProvider {
public:
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_VIRTUAL_DESTRUCTOR(WeatherInfo);

protected:
    void FindInformation() override;

private:
    std::tuple<std::string, std::string, std::string, std::string> GetWeatherInfo();
};

} // namespace mmotd::information
