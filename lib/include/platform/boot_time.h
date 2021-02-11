// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <chrono>
#include <optional>
#include <string>

namespace mmotd::platform {

std::optional<std::chrono::system_clock::time_point> GetBootTime();

} // namespace mmotd::platform
