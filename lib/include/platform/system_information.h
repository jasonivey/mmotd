// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <string>
#include <tuple>
#include <vector>

namespace mmotd::platform {

using SystemInformationDetail = std::tuple<std::string, std::string>;
using SystemInformationDetails = std::vector<SystemInformationDetail>;

SystemInformationDetails GetSystemInformationDetails();

} // namespace mmotd::platform
