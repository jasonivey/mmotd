// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "common/assertion/include/assertion.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iterator>
#include <limits>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace mmotd::algorithms {

template<typename... Args>
inline constexpr void unused(Args &&...) noexcept {}

template<class BidIter, class T>
inline BidIter find_last(BidIter first, BidIter last, T value) {
    for (BidIter i = last; i != first;) {
        if (*(--i) == value) {
            return i;
        }
    }
    return last;
}

template<class BidIter, class Pred>
inline BidIter find_last_if(BidIter first, BidIter last, Pred pred) {
    for (BidIter i = last; i != first;) {
        if (pred(*(--i))) {
            return i;
        }
    }
    return last;
}

//
// transform_if: the basic description of `std::transform` is with a container
//  of objects the algorithm will call the `unary` function, `func`, to
//  transform the object to type `OutIter::value_type` for inserting into
//  `OutIter`.
//  Now if the user only wanted a container of these transformed objects which
//   satisfied a particular requirement the `transform_if` algorithm would be
//   needed.
//  In `tranform_if` the algorithm will pass over each value in the input range
//   and pass them to the `predicate`.  If the `predicate` returns `true` then
//   the value will be passed to the unary `func` for transformation to the type
//   `OutIter::value_type` and inserted into `OutIter`.
//
// Complexity: At most, last - first applications of the `predicate` and possibly
//  last - first applications of `func`.
//
template<class InIter, class OutIter, class Pred, class Func>
inline OutIter transform_if(InIter first, InIter last, OutIter out, Pred pred, Func func) {
    for (; first != last; ++first) {
        if (pred(*first)) {
            *(out++) = func(*first);
        }
    }
    return out;
}

//
// erase_if: given a container of elements the algorithm will test each item for
//  whether to delete it using the provided `predicate`.
//
// Usage: provides an algorithm for removing and erasing elements within
//  associative containers where `std::remove_if` does not make sense.  e.g. how
//  can you move a removed element in a `std::map` to the `end` of the range?
//
// Complexity: At most, last - first applications of the `predicate` and possibly
//  last - first applications of `container::erase(iterator)`.
//
template<class Container, class Pred>
inline size_t erase_if(Container &container, Pred pred) {
    auto start_size = container.size();
    for (auto i = std::begin(container), last = std::end(container); i != last;) {
        if (pred(*i)) {
            i = container.erase(i);
        } else {
            ++i;
        }
    }
    CHECKS(start_size >= container.size(), "erase_if should never make the container larger");
    return start_size - container.size();
}

//
// consecutive_find: given a container of elements it will find spans of identical
//  elements and populate the `OutIter` with `pair`s of `{element count, element}`
//  also known as a `run` of elements.
//
// Complexity: At most, last - first applications of the `predicate`
//
template<class InIter, class OutIter>
inline OutIter consecutive_find(InIter first, InIter last, OutIter out) {
    auto predicate = std::equal_to<typename std::iterator_traits<InIter>::value_type>{};
    return consecutive_find(first, last, out, predicate);
}

template<class InIter, class OutIter, class Pred>
inline OutIter consecutive_find(InIter first, InIter last, OutIter out, Pred pred) {
    if (std::distance(first, last) == 0) {
        return out;
    }
    auto length = typename std::iterator_traits<InIter>::difference_type{1};
    for (auto i = first + 1; i != last; ++i) {
        if (pred(*(i - 1), *i)) {
            if (i + 1 == last) {
                *(out++) = std::make_pair(i - length, i);
            }
            ++length;
        } else {
            *(out++) = std::make_pair(i - length, i - 1);
            length = 1;
        }
    }
    return out;
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

//
// collect_if: based off of the idea behind `std::any_of`.  The difference between
//  that collection `predicate` and this `copy-like-algorithm` is simply that, if
//  the `predicate` holds true it will copy the value.
//
// Details: the range from `first` to `last` is iterated and the `predicate` is called
//  on each `value`.  If the `predicate` passes, then the unary `func` is called with
//  the `value` and the `result` is inserted into `out`.
//
// Complexity: At most, last - first applications of the `predicate` and possibly
//  last - first applications of `func`.
//
template<class InIter, class OutIter, class Pred, class Func>
inline OutIter collect_if(InIter first, InIter last, OutIter out, Pred pred) {
    for (; first != last; ++first) {
        if (pred(*first)) {
            *(out++) = *first;
        }
    }
    return out;
}

template<class InIter, class OutIter, class Pred, class Func>
inline OutIter collect_if(InIter first, InIter last, OutIter out, Pred pred, Func func) {
    for (; first != last; ++first) {
        if (pred(*first)) {
            *(out++) = func(*first);
        }
    }
    return out;
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

template<typename T, class U>
constexpr bool value_outside_range(U value, T min, T max_plus_one) noexcept {
    using namespace std;
    return cmp_less(value, min) || cmp_greater_equal(value, max_plus_one);
}

} // namespace mmotd::algorithms
