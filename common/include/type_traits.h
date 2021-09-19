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

// define type_trait for testing whether a type is a std::time_point
template<typename T>
struct is_time_point : std::false_type {};

template<class Clock, class Duration>
struct is_time_point<std::chrono::time_point<Clock, Duration>> : std::true_type {};

// define type_trait for testing whether a type is a container
template<typename T, typename _ = void>
struct is_container : std::false_type {};

template<typename T>
struct is_container<T,
                    std::void_t<typename T::value_type,
                                typename T::size_type,
                                typename T::allocator_type,
                                typename T::iterator,
                                typename T::const_iterator,
                                decltype(std::declval<T>().size()),
                                decltype(std::declval<T>().begin()),
                                decltype(std::declval<T>().end()),
                                decltype(std::declval<T>().cbegin()),
                                decltype(std::declval<T>().cend())>> : public std::true_type {};

} // namespace mmotd::type_traits
