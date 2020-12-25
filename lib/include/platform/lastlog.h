// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace mmotd::platform {

using LastLogDetail = std::tuple<std::string, std::string>;
using LastLogDetails = std::vector<LastLogDetail>;

LastLogDetails GetLastLogDetails();

} // namespace mmotd::platform
