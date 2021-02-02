// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/information_objects.h"

#include <string>
#include <tuple>
#include <vector>

namespace mmotd::platform {

using Detail = std::tuple<std::string, std::string>;
using Details = std::vector<Detail>;

Details GetMemoryDetails();

} // namespace mmotd::platform
