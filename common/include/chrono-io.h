// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <chrono>
#include <string>

namespace mmotd::chrono::io {

std::string to_string(std::chrono::system_clock::time_point time_point, std::string chrono_format);

} // namespace mmotd::chrono::io
