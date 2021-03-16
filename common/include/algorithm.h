// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace mmotd::algorithms {

template<typename... Args>
inline void unused(Args &&...) noexcept {
}

template<class InIter, class OutIter, class Pred, class Func>
inline OutIter transform_if(InIter first, InIter last, OutIter out, Pred pred, Func func) {
    for (; first != last; ++first, ++out) {
        if (pred(*first)) {
            *out = func(*first);
        }
    }
    return out;
}

template<class Key, class T, class Hash, class Equal, class Alloc, class Pred>
inline auto erase_if(std::unordered_map<Key, T, Hash, Equal, Alloc> &container, Pred predicate) {
    auto old_size = container.size();
    for (auto i = std::begin(container), last = std::end(container); i != last;) {
        if (predicate(*i)) {
            i = container.erase(i);
        } else {
            ++i;
        }
    }
    return old_size - container.size();
}

//
// join algorithm which is similar to `boost::join` except it will use `Func` to convert objects
//  from `Container::value_type` to string.  The container must be a container-of-containers.
//  For example, `vector<int>`, `list<double>` or `map<chrono::system_clock::time_point, string>`.
//  Following on with the example of `vector<int>` the `Container::value_type` will be `int`.  This
//  makes the signature of `Func`:
//  `string Func(Container::value_type)` or more precisely:
//  `string Func(int)` which could be something a call like the following:
//  `auto ints = vector<int>(size_t{10});`
//  `iota(begin(ints), end(ints), ints.front());`
//  `cout << "ints: " << join(ints, ", ", [](auto i) { return to_string(i); });`
//
template<class Container, class Seperator, class Func>
inline std::string join(const Container &container, Seperator seperator, Func func) {
    auto output = std::string{};
    auto i = std::begin(container);
    auto end = std::end(container);
    if (i != end) {
        output += func(*i);
        ++i;
    }
    for (; i != end; ++i) {
        output += seperator;
        output += func(*i);
    }
    return output;
}

template<typename Iter, typename Pred, typename Func>
void collect_if(Iter first, Iter last, Pred predicate, Func func) {
    for (; first != last; ++first) {
        if (predicate(*first)) {
            func(*first);
        }
    }
}

#if !defined(__cpp_lib_integer_comparison_functions)
template<class T, class U>
constexpr bool cmp_less(T t, U u) noexcept {
    using UnsignedTypeT = std::make_unsigned_t<T>;
    using UnsignedTypeU = std::make_unsigned_t<U>;
    if constexpr (std::is_signed_v<T> == std::is_signed_v<U>) {
        return t < u;
    } else if constexpr (std::is_signed_v<T>) {
        return t < 0 ? true : UnsignedTypeT{t} < u;
    } else {
        return u < 0 ? false : t < UnsignedTypeU{u};
    }
}

template<class T, class U>
constexpr bool cmp_greater(T t, U u) noexcept {
    return cmp_less(u, t);
}

template<class T, class U>
constexpr bool cmp_less_equal(T t, U u) noexcept {
    return !cmp_greater(t, u);
}

template<class T, class U>
constexpr bool cmp_greater_equal(T t, U u) noexcept {
    return !cmp_less(t, u);
}
#endif

template<typename T, class U>
constexpr bool value_in_range(U value, T min, T max_plus_one) noexcept {
    using namespace std;
    return cmp_greater_equal(value, min) && cmp_less(value, max_plus_one);
}

} // namespace mmotd::algorithms
