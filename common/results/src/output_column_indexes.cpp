// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/algorithm.h"
#include "common/results/include/output_column_indexes.h"

#include <algorithm>

using namespace mmotd::algorithms;
using namespace std;

using fmt::format;
using mmotd::results::data::ENTIRE_LINE, mmotd::results::data::ENTIRE_LINE_REPR;

namespace mmotd::results {

bool ColumnIndexes::contains(ColumnIndex i) const noexcept {
    return std::find(indexes_.begin(), indexes_.end(), i) != indexes_.end();
}

string ColumnIndexes::to_string() const {
    return join(indexes_, ", ", [](auto index) {
        return index == ENTIRE_LINE ? string(ENTIRE_LINE_REPR) : std::to_string(index);
    });
}

void ColumnIndexes::insert(ColumnIndex value) noexcept {
    if (auto i = std::find(indexes_.begin(), indexes_.end(), value); i == indexes_.end()) {
        indexes_.push_back(value);
        sort();
    }
}

void ColumnIndexes::sort(container &indexes) noexcept {
    std::sort(indexes.begin(), indexes.end(), [](auto a, auto b) {
        return a == ENTIRE_LINE ? true : b == ENTIRE_LINE ? false : a < b;
    });
}

void ColumnIndexes::sort() noexcept {
    ColumnIndexes::sort(indexes_);
}

} // namespace mmotd::results
