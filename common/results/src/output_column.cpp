// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/information.h"
#include "common/include/informations.h"
#include "common/include/logging.h"
#include "common/results/include/output_column.h"
#include "common/results/include/output_row.h"
#include "common/results/include/output_row_number_sentinals.h"
#include "common/results/include/template_column_items.h"

#include <boost/uuid/uuid_io.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>

using fmt::format;
using mmotd::information::Informations;
using mmotd::results::data::TemplateColumnItem;

using namespace std;

static constexpr int INVALID_ROW = std::numeric_limits<int>::max();

namespace mmotd::results {

void Column::AddRow(TemplateColumnItem item, int &increment_row, size_t count) {
    for (auto i = size_t{0}; i != count; ++i) {
        auto new_item = TemplateColumnItem{item};
        if (i > size_t{0}) {
            increment_row += static_cast<int>(max(size(item.name), size(item.value)));
            new_item.repeatable_index = static_cast<int>(i);
        }
        new_item.row_index += increment_row;
        rows_.emplace_back(Row{new_item});
    }
}

void Column::SetPositionIndex(PositionIndex position_index) {
    position_index_ = position_index;
    for_each(begin(rows_), end(rows_), [position_index](auto &row) { row.SetPositionIndex(position_index); });
}

void Column::AddNamesValues(const Informations &informations, PositionIndex position_index) {
    SetPositionIndex(position_index);
    for (auto &row : rows_) {
        row.SetNameValue(informations);
    }
    for (const auto &row : rows_) {
        LOG_VERBOSE("row[{}] = [{}] {}", row.GetRowNumber(), fmt::ptr(&row), row.item_to_string());
    }
}

bool Column::BalanceNameValueSize() {
    auto modified = false;
    for (auto i = begin(rows_); i != end(rows_);) {
        if (!(*i).BalanceNameValueSize()) {
            i = rows_.erase(i);
            modified = true;
        } else {
            ++i;
        }
    }
    return modified;
}

bool Column::ContainsRow(RowId row_id) const {
    auto i = find_if(begin(rows_), end(rows_), [row_id](auto &row) { return row.GetId() == row_id; });
    return i != end(rows_);
}

bool Column::ContainsRow(int row_number) const {
    auto i = find_if(begin(rows_), end(rows_), [row_number](auto &row) { return row.GetRowNumber() == row_number; });
    return i != end(rows_);
}

bool Column::HasPreviousRow(RowId row_id) const {
    auto i = find_if(begin(rows_), end(rows_), [row_id](auto &row) { return row.GetId() == row_id; });
    return i != begin(rows_) && i != end(rows_);
}

const Row &Column::GetPreviousRow(RowId row_id) const {
    auto i = find_if(begin(rows_), end(rows_), [row_id](auto &row) { return row.GetId() == row_id; });
    CHECKS(i != end(rows_), "row id '{}' was not found", row_id);
    return *(i - 1);
}

Row &Column::GetPreviousRow(RowId row_id) {
    auto i = find_if(begin(rows_), end(rows_), [row_id](auto &row) { return row.GetId() == row_id; });
    CHECKS(i != end(rows_), "row id '{}' was not found", row_id);
    return *(i - 1);
}

const Row &Column::GetRow(RowId row_id) const {
    auto i = find_if(begin(rows_), end(rows_), [row_id](auto &row) { return row.GetId() == row_id; });
    CHECKS(i != end(rows_), "row id '{}' was not found", row_id);
    return *i;
}

Row &Column::GetRow(RowId row_id) {
    auto i = find_if(begin(rows_), end(rows_), [row_id](auto &row) { return row.GetId() == row_id; });
    CHECKS(i != end(rows_), "row id '{}' was not found", row_id);
    return *i;
}

Row &Column::GetRow(int row_number) {
    auto i = find_if(begin(rows_), end(rows_), [row_number](auto &row) { return row.GetRowNumber() == row_number; });
    CHECKS(i != end(rows_), "row number '{}' was not found", row_number);
    return *i;
}

const Row &Column::GetRow(int row_number) const {
    auto i =
        find_if(begin(rows_), end(rows_), [row_number](const auto &row) { return row.GetRowNumber() == row_number; });
    CHECKS(i != end(rows_), "row number '{}' was not found", row_number);
    return *i;
}

void Column::CollapseRows() {
    auto prev_row = -1;
    for (auto &row : rows_) {
        if (prev_row != -1 && prev_row != row.GetRowNumber()) {
            row.UpdateRowNumber(prev_row);
        }
        prev_row = row.GetRowNumber() + static_cast<int>(row.GetHeight());
    }
}

RowNumberSentinals Column::GetFirstLastRowNumbers() const {
    auto first_row_number = INVALID_ROW;
    auto last_row_number = 0;
    for (const auto &row : rows_) {
        auto row_number = row.GetRowNumber();
        first_row_number = min(first_row_number, row_number);
        last_row_number = max(last_row_number, row_number);
    }
    return RowNumberSentinals(first_row_number, last_row_number);
}

size_t Column::GetColumnCount() const {
    auto count = size_t{0};
    for (const auto &row : rows_) {
        auto row_column_count = row.GetColumnCount();
        count = max(count, row_column_count);
    }
    return count;
}

} // namespace mmotd::results
