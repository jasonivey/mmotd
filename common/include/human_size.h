// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/type_traits.h"

#include <array>
#include <string>
#include <type_traits>

#include <fmt/format.h>

namespace mmotd::algorithm::string {

// Algorithm and code modified from https://gist.github.com/dgoguerra/7194777 thanks @dgoguerra
template<class T,
         typename = std::enable_if_t<(std::is_integral<T>::value && !mmotd::type_traits::is_bool<T>::value) ||
                                     std::is_floating_point<T>::value>>
std::string to_human_size(T number) {
    const auto suffixes = std::array<std::string, 5>{"B", "KB", "MB", "GB", "TB"};

    auto bytes = number;
    auto floating_point_bytes = static_cast<double>(bytes);
    constexpr const auto KILO_BYTE = T{1024};

    auto suffix = suffixes.front();
    for (auto i : suffixes) {
        if (bytes < KILO_BYTE) {
            suffix = i;
            break;
        }
        floating_point_bytes = bytes / static_cast<double>(KILO_BYTE);
        bytes /= KILO_BYTE;
    }
    return fmt::format(FMT_STRING("{:.02f} {}"), floating_point_bytes, suffix);
}

} // namespace mmotd::algorithm::string
