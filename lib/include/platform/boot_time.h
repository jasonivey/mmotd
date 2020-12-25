// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <optional>
#include <string>

namespace mmotd::platform {

std::optional<std::string> GetBootTime();

} // namespace mmotd
