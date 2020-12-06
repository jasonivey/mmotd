#pragma once
#include <type_traits>

namespace mmotd::type_traits {

// define the trait as false for every type T
template<typename T>
struct is_bool : std::false_type {};

// now only define the trait as true for 'bool'
template<>
struct is_bool<bool> : std::true_type {};

} // namespace mmotd::type_traits
