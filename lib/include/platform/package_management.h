// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <string>

namespace mmotd::platform::package_management {

std::string GetUpdateDetails();
std::string GetRebootRequired();

} // namespace mmotd::platform::package_management
