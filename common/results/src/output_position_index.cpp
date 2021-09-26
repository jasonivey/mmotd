// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/algorithm.h"
#include "common/results/include/output_column_indexes.h"
#include "common/results/include/output_position_index.h"

#include <algorithm>
#include <array>
#include <vector>

using namespace mmotd::algorithms;
using namespace std;

using mmotd::results::data::ENTIRE_LINE;

namespace {

vector<int> GetIndexesWithoutEntireLine(vector<int> indexes) {
    auto i = remove(begin(indexes), end(indexes), ENTIRE_LINE);
    indexes.erase(i, end(indexes));
    return indexes;
}

vector<int> GetIndexesWithoutEntireLine(vector<int> indexes, ptrdiff_t index) {
    PRECONDITIONS(static_cast<size_t>(index) < size(indexes), "index must be less than the size of the container");
    auto i = remove(begin(indexes), begin(indexes) + index, ENTIRE_LINE);
    indexes.erase(i, begin(indexes) + index);
    return indexes;
}

} // namespace

namespace mmotd::results {

PositionIndex::PositionIndex(const ColumnIndexes &indexes, size_t index) :
    PositionIndex({indexes.cbegin(), indexes.cend()}, index) {
}

PositionIndex::PositionIndex(const vector<int> &indexes, size_t index) : index_(index) {
    PRECONDITIONS(index < size(indexes), "index must be less than the size of the container");
    column_ = static_cast<int>(indexes[index]);
    auto container_without_entire_line = GetIndexesWithoutEntireLine(indexes);
    if (size(container_without_entire_line) == 1 || column_ == ENTIRE_LINE) {
        position_ = Position::FirstAndLast;
    } else {
        index -= (size(indexes) - size(GetIndexesWithoutEntireLine(indexes, static_cast<ptrdiff_t>(index))));
        if (index == 0) {
            position_ = Position::First;
        } else if (index + 1 == size(container_without_entire_line)) {
            position_ = Position::Last;
        } else {
            position_ = Position::Middle;
        }
    }
}

size_t to_size_t(PositionIndex::Position position) {
    return static_cast<size_t>(position);
}

PositionIndex::Position operator&(PositionIndex::Position a, PositionIndex::Position b) {
    return static_cast<PositionIndex::Position>(to_size_t(a) & to_size_t(b));
}

PositionIndex::Position operator|(PositionIndex::Position a, PositionIndex::Position b) {
    return static_cast<PositionIndex::Position>(to_size_t(a) | to_size_t(b));
}

size_t PositionIndex::GetIndex() const noexcept {
    if (position_ == Position::FirstAndLast || index_ == 0) {
        return size_t{0};
    } else {
        return index_ - 1;
    }
}

string PositionIndex::to_string() const {
    PRECONDITIONS(IsPositionValid(true), "position index must be one of the Position values");
    switch (position_) {
        case Position::First:
            return "first";
        case Position::Middle:
            return "middle";
        case Position::Last:
            return "last";
        case Position::FirstAndLast:
            return "first-and-last";
        case Position::Invalid:
        default:
            return "invalid";
    }
}

bool PositionIndex::IsStartOfLine() const {
    PRECONDITIONS(IsPositionValid(true), "position index must be one of the Position values");
    return IsPositionOneOf(position_, PositionIndex::Position::First | PositionIndex::Position::FirstAndLast);
}

bool PositionIndex::IsEntireLine() const {
    PRECONDITIONS(IsPositionValid(true), "position index must be one of the Position values");
    return position_ == PositionIndex::Position::FirstAndLast;
}

bool PositionIndex::IsFirstColumn() const {
    PRECONDITIONS(IsPositionValid(true), "position index must be one of the Position values");
    return position_ == PositionIndex::Position::First;
}

bool PositionIndex::IsMiddleColumn() const {
    PRECONDITIONS(IsPositionValid(true), "position index must be one of the Position values");
    return position_ == PositionIndex::Position::Middle;
}

bool PositionIndex::IsLastColumn() const {
    PRECONDITIONS(IsPositionValid(true), "position index must be one of the Position values");
    return position_ == PositionIndex::Position::Last;
}

bool PositionIndex::IsEndOfLine() const {
    PRECONDITIONS(IsPositionValid(true), "position index must be one of the Position values");
    return IsPositionOneOf(position_, PositionIndex::Position::FirstAndLast | PositionIndex::Position::Last);
}

bool PositionIndex::IsMultipleColumnLine() const {
    PRECONDITIONS(IsPositionValid(true), "position index must be one of the Position values");
    return IsPositionOneOf(position_,
                           PositionIndex::Position::First | PositionIndex::Position::Middle |
                               PositionIndex::Position::Last);
}

bool PositionIndex::IsPositionOneOf(Position position, Position flag) noexcept {
    return (position & flag) != Position::Invalid && IsPositionValid(position & flag);
}

bool PositionIndex::IsPositionValid(bool include_invalid) const noexcept {
    return IsPositionValid(position_, include_invalid);
}

bool PositionIndex::IsPositionValid(Position position, bool include_invalid) noexcept {
    static constexpr const auto positions =
        array<Position, 4>{Position::First, Position::Middle, Position::Last, Position::FirstAndLast};
    static constexpr const auto all_positions = array<Position, 5>{Position::Invalid,
                                                                   Position::First,
                                                                   Position::Middle,
                                                                   Position::Last,
                                                                   Position::FirstAndLast};
    if (include_invalid) {
        return find(begin(all_positions), end(all_positions), position) != end(positions);
    } else {
        return find(begin(positions), end(positions), position) != end(positions);
    }
}

} // namespace mmotd::results
