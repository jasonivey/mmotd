// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace mmotd::platform {

struct LastLoginDetails {
    std::string summary;
    std::chrono::system_clock::time_point log_in;
    std::chrono::system_clock::time_point log_out;
};

LastLoginDetails GetLastLogDetails();

} // namespace mmotd::platform
