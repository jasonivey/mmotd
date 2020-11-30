// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <chrono>
#include <string>
#include <string_view>

namespace mmotd { namespace chrono { namespace io {

std::string to_string(std::chrono::system_clock system_clock, std::string_view chrono_format);

}}} // namespace mmotd::chrono::io

