// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"
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
inline std::string to_human_size(T number) {
    const auto suffixes = std::array<std::string, 5>{"B", "KB", "MB", "GB", "TB"};

    auto bytes = number;
    auto floating_point_bytes = static_cast<double>(bytes);
    constexpr auto KILO_BYTE = T{1024};

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

namespace mmotd::algorithm {

// A class which wraps the above conversion behaviour.  This allows perserving
//  and storing the initial raw value.  Then, when needed, that raw value can
//  be converted to the human readable format and returned via a `std::string`.
template<class T,
         typename = std::enable_if_t<(std::is_integral<T>::value && !mmotd::type_traits::is_bool<T>::value) ||
                                     std::is_floating_point<T>::value>>
class HumanNumber {
public:
    explicit HumanNumber(T value) : value_(value) {}
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(HumanNumber);

    void value(T new_value) { value_ = new_value; }
    T value() const noexcept { return value_; }

    std::string to_string() const noexcept { return mmotd::algorithm::string::to_human_size(value_); }

private:
    T value_ = T{};
};

} // namespace mmotd::algorithm
