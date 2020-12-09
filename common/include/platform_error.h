// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <string>

namespace mmotd::platform::error {

std::string to_string(int error_value);
std::string errno_to_string();

} // namespace mmotd::platform::error
