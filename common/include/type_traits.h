#pragma once
#include <chrono>
#include <type_traits>

namespace mmotd::type_traits {

// define the trait as false for every type T
template<typename T>
struct is_bool : std::false_type {};

// now only define the trait as true for 'bool'
template<>
struct is_bool<bool> : std::true_type {};

template<typename T>
struct is_time_point : std::false_type {};

template<class Clock, class Duration>
struct is_time_point<std::chrono::time_point<Clock, Duration>> : std::true_type {};

} // namespace mmotd::type_traits
