// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <chrono>
#include <optional>
#include <string>

namespace mmotd::chrono::io {

std::string to_string(std::chrono::system_clock::time_point time_point, std::string chrono_format);

enum class FromStringFormat {
    TimeFormat, // 18:47:01 or \d{2}:\d{2}\d{2}
};

std::optional<std::chrono::system_clock::time_point> from_string(std::string date_time_str,
                                                                 FromStringFormat format_type);

} // namespace mmotd::chrono::io
