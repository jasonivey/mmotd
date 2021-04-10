// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "common/include/big_five_macros.h"

#include <cstdint>
#include <string>
#include <vector>

namespace mmotd::results {

class ColumnIndexes;

class PositionIndex {
public:
    enum class Position : std::size_t {
        Invalid = 0,
        First = 1 << 0,
        Middle = 1 << 1,
        Last = 1 << 2,
        FirstAndLast = 1 << 3
    };

    PositionIndex(const ColumnIndexes &indexes, std::size_t index);
    PositionIndex(const std::vector<int> &indexes, std::size_t index);
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(PositionIndex);

    Position GetPosition() const noexcept { return position_; }
    std::string to_string() const;
    int GetColumnNumber() const noexcept { return column_; }
    std::size_t GetIndex() const noexcept;

    bool IsStartOfLine() const;
    bool IsEntireLine() const;
    bool IsFirstColumn() const;
    bool IsMiddleColumn() const;
    bool IsLastColumn() const;
    bool IsEndOfLine() const;
    bool IsMultipleColumnLine() const;

    bool IsPositionValid(bool include_invalid = false) const noexcept;

private:
    static bool IsPositionOneOf(Position position, Position flag) noexcept;
    static bool IsPositionValid(Position position, bool include_invalid = false) noexcept;
    static std::vector<int> GetPositionContainerWithoutEntireLineUpToIndex(std::vector<int> container,
                                                                           std::size_t index);
    static std::vector<int> GetPositionContainerWithoutEntireLine(std::vector<int> container);

    Position position_ = Position::Invalid;
    std::size_t index_ = 0;
    int column_ = -1;
};

} // namespace mmotd::results
