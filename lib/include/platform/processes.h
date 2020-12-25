// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <cstdint>
#include <optional>

namespace mmotd::platform {

std::optional<std::size_t> GetProcessCount();

} // namespace mmotd::platform
