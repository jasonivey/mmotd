// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

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

} // namespace mmotd::algorithms
