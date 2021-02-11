// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <string>
#include <tuple>
#include <vector>

namespace mmotd::platform {

struct SwapDetails {
    uint64_t total = 0ull;
    uint64_t free = 0ull;
    double percent_used = 0.0;
    bool encrypted = false;
};

SwapDetails GetSwapDetails();

} // namespace mmotd::platform
