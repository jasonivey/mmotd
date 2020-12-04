// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <array>
#include <fmt/format.h>

namespace mmotd::algorithm::string {

// Algorithm and code modified from https://gist.github.com/dgoguerra/7194777 thanks @dgoguerra
template<typename T>
inline std::string to_human_size(T number) {
    const auto suffixes = std::array<std::string, 5>{"B", "KB", "MB", "GB", "TB"};

    auto bytes = number;
    auto floating_point_bytes = static_cast<double>(bytes);

    auto suffix = suffixes.front();
    for (auto i : suffixes) {
        if (bytes < 1024) {
            suffix = i;
            break;
        }
        floating_point_bytes = bytes / 1024.0;
        bytes /= 1024;
    }
    return fmt::format("{:.02f} {}", floating_point_bytes, suffix);
}

} // namespace mmotd::algorithm::string
