// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <iterator>
#include <unordered_map>

namespace mmotd::algorithms {

template<class InputIterator, class OutputIterator, class Predicate, class UnaryOperation>
inline OutputIterator
transform_if(InputIterator first, InputIterator last, OutputIterator out, Predicate pred, UnaryOperation op) {
    for (; first != last; ++first, (void)++out) {
        if (pred(*first)) {
            *out = op(*first);
        }
    }
    return out;
}

template<class Key, class T, class Hash, class KeyEqual, class Alloc, class Pred>
typename std::unordered_map<Key, T, Hash, KeyEqual, Alloc>::size_type
erase_if(std::unordered_map<Key, T, Hash, KeyEqual, Alloc> &container, Pred predicate) {
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

} // namespace mmotd::algorithms
