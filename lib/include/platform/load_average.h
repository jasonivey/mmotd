// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <optional>
#include <string>
#include <tuple>

namespace mmotd::platform {

using LoadAverageDetails = std::tuple<int32_t, double>;

LoadAverageDetails GetLoadAverageDetails();

} // namespace mmotd::platform
