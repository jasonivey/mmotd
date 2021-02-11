// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <string>
#include <tuple>
#include <vector>

namespace mmotd::platform {

struct MemoryDetails {
    uint64_t total = 0ull;
    uint64_t available = 0ull;
    double percent_used = 0.0;
    uint64_t used = 0ull;
    uint64_t free = 0ull;
    uint64_t active = 0ull;
    uint64_t inactive = 0ull;
    uint64_t wired = 0ull;
};

MemoryDetails GetMemoryDetails();

} // namespace mmotd::platform
