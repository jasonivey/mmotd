// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
// #include "common/assertion/include/assertion.h"
// #include "common/results/include/output_position_index.h"
#include "common/assertion/include/assertion.h"
#include "common/include/information.h"
#include "common/include/information_decls.h"
#include "common/include/informations.h"
#include "common/include/logging.h"
#include "common/results/include/output_column.h"
#include "common/results/include/output_frame.h"
#include "common/results/include/output_row.h"
#include "common/results/include/output_row_number_sentinals.h"
#include "common/results/include/output_table.h"
#include "common/results/include/template_column_items.h"
#include "common/results/include/template_string.h"

#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <iterator>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/uuid/uuid_io.hpp>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

using fmt::format;
using mmotd::information::InformationId;
using mmotd::information::Informations;
using mmotd::results::data::ENTIRE_LINE;
using mmotd::results::data::ENTIRE_LINE_REPR;
using mmotd::results::data::OutputSettings;
using mmotd::results::data::TemplateColumnItem;
using mmotd::results::data::TemplateColumnItems;

using namespace std;

// namespace {

// string column_index_to_string(mmotd::results::ColumnIndex index) {
//     return index == ENTIRE_LINE ? string(ENTIRE_LINE_REPR) : std::to_string(index);
// }

// } // namespace

namespace mmotd::results {

ColumnIndexes Frame::GetColumnIndexes() const {
    auto column_indexes = ColumnIndexes{};
    for_each(begin(columns_), end(columns_), [&column_indexes](const auto &column) {
        const auto &[index, _] = column;
        column_indexes.insert(index);
    });
    return column_indexes;
}

bool Frame::ContainsColumn(ColumnIndex index) const {
    auto i = find_if(begin(columns_), end(columns_), [index](const auto &column_value) {
        const auto &[idx, _] = column_value;
        return idx == index;
    });
    return i != end(columns_);
}

const Column &Frame::GetColumn(ColumnIndex index) const {
    auto i = find_if(begin(columns_), end(columns_), [index](const auto &column_value) {
        const auto &[idx, _] = column_value;
        return idx == index;
    });
    if (i == end(columns_)) {
        THROW_OUT_OF_RANGE("column index {} not found in template columns", index);
    }
    const auto &[_, column] = *i;
    return column;
}

Column &Frame::GetColumn(ColumnIndex index) {
    auto i = find_if(begin(columns_), end(columns_), [index](const auto &column_value) {
        const auto &[idx, _] = column_value;
        return idx == index;
    });
    if (i == end(columns_)) {
        THROW_OUT_OF_RANGE("column index {} not found in template columns", index);
    }
    auto &[_, column] = *i;
    return column;
}

const Column &Frame::GetColumn(RowId row_id) const {
    for (auto column_index : GetColumnIndexes()) {
        const auto &column = GetColumn(column_index);
        if (column.ContainsRow(row_id)) {
            return column;
        }
    }
    THROW_OUT_OF_RANGE("row id {} not found in any columns", row_id);
}

Column &Frame::GetColumn(RowId row_id) {
    for (auto column_index : GetColumnIndexes()) {
        auto &column = GetColumn(column_index);
        if (column.ContainsRow(row_id)) {
            return column;
        }
    }
    THROW_OUT_OF_RANGE("row id {} not found in any columns", row_id);
}

size_t Frame::GetRowValueInformationIdCount(const string &value, const Informations &informations) const {
    auto id = TemplateString{value, fmt::text_style{}}.GetFirstInformationId(informations);
    if (id == InformationId::ID_INVALID_INVALID_INFORMATION) {
        return size_t{0};
    }
    return informations.count(id);
}

void Frame::AddRow(TemplateColumnItem item, int &increment_row, size_t count) {
    using namespace mmotd::results::data;
    if (!ContainsColumn(item.column)) {
        columns_.push_back(make_pair(item.column, Column{}));
    }
    GetColumn(item.column).AddRow(item, increment_row, count);
}

void Frame::AddRowsColumns(TemplateColumnItems items, const Informations &informations) {
    sort(begin(items), end(items), [](const auto &a, const auto &b) { return a.row_index < b.row_index; });
    auto increment_row = 0;
    for (const auto &item : items) {
        auto item_value = empty(item.value) ? string{} : string(item.value.front());
        if (empty(item_value)) {
            continue;
        } else if (!item.is_optional && !item.is_repeatable) {
            AddRow(item, increment_row);
        } else {
            auto item_count = GetRowValueInformationIdCount(item_value, informations);
            if (item_count == size_t{0} && item.is_optional) {
                continue;
            } else if (item_count > size_t{1} && item.is_repeatable) {
                AddRow(item, increment_row, item_count);
            } else {
                AddRow(item, increment_row);
            }
        }
    }
}

void Frame::AddNamesValues(const Informations &informations) {
    const auto &indexes = GetColumnIndexes();
    for (auto i = size_t{0}; i != size(indexes); ++i) {
        const auto index = indexes[i];
        auto position_index = PositionIndex(indexes, i);
        GetColumn(index).AddNamesValues(informations, position_index);
    }
    for (auto index : indexes) {
        GetColumn(index).BalanceNameValueSize();
    }
}

// This increments for the row height - 1, assuming that the for loop has a built-in ++i on every
//  iteration.
template<typename T, typename It>
static void IncrementRowNumberSentinalsForHeight(const T &container, It &iter, size_t row_height) {
    while (--row_height > 0) {
        if (iter != end(container)) {
            ++iter;
        }
    }
}

void Frame::RemoveEmptyRows() {
    auto empty_row_count = 0;
    auto row_numbers = GetFirstLastRowNumbers();
    for (auto i = begin(row_numbers); i != end(row_numbers); ++i) {
        auto row_number = *i;
        LOG_VERBOSE("inspecting row: {}, empty rows: {}", row_number, empty_row_count);
        auto row_ids = GetRowIds(row_number);
        if (empty(row_ids)) {
            ++empty_row_count;
            continue;
        }
        if (empty_row_count == 0) {
            CHECKS(!empty(row_ids), "must be dealing with a non-empty row at this point");
            IncrementRowNumberSentinalsForHeight(row_numbers, i, GetRowHeight(row_number));
            continue;
        }
        auto row_height = size_t{1};
        for (auto row_id : row_ids) {
            auto &row = GetRow(row_id);
            if (row.GetRowNumber() >= empty_row_count) {
                auto updated_row_number = row.GetRowNumber() - empty_row_count;
                auto height = row.UpdateRowNumber(updated_row_number);
                row_height = max(row_height, height);
            }
        }
        IncrementRowNumberSentinalsForHeight(row_numbers, i, row_height);
    }
}

void Frame::ReindexRows() {
    auto row_numbers_row_ids = unordered_map<int, RowIds>{};
    auto row_numbers_row_ids_indexes = vector<int>{};
    auto row_numbers = GetFirstLastRowNumbers();
    for (auto row_number : row_numbers) {
        row_numbers_row_ids.insert(make_pair(row_number, GetRowIds(row_number)));
        row_numbers_row_ids_indexes.push_back(row_number);
    }
    sort(begin(row_numbers_row_ids_indexes), end(row_numbers_row_ids_indexes));

    auto current_row_number = 0;
    for (auto row_number : row_numbers_row_ids_indexes) {
        auto current_row_height = size_t{1};
        auto row_ids = row_numbers_row_ids[row_number];
        for (auto i = begin(row_ids); i != end(row_ids); ++i) {
            auto &row = GetRow(*i);
            auto row_height = row.GetHeight();
            if (row.GetRowNumber() != current_row_number) {
                row_height = row.UpdateRowNumber(current_row_number);
            }
            if (size(row_ids) == 1) {
                current_row_height = max(current_row_height, row_height);
            }
        }
        current_row_number += static_cast<int>(current_row_height);
    }

    RemoveEmptyRows();
}

void Frame::CollapseColumnRows() {
    auto indexes = GetColumnIndexes();
    for (auto index : indexes) {
        if (index != ENTIRE_LINE) {
            GetColumn(index).CollapseRows();
        }
    }
}

void Frame::PrepareForOutput(OutputSettings settings) {
    if (settings.collapse_column_rows) {
        CollapseColumnRows();
    }
    ReindexRows();
    table_type_ = settings.table_type;
}

string Frame::CreateTable() const {
    auto column_count = GetColumnCount(true);
    auto table = Table(table_type_, column_count);
    for (auto i : GetFirstLastRowNumbers()) {
        const auto &rows = GetRows(i);
        for (const auto &row : rows) {
            table.WriteRow(row);
        }
    }
    return table.to_string();
}

RowNumberSentinals Frame::GetFirstLastRowNumbers() const {
    auto first_row_number = std::numeric_limits<int>::max();
    auto last_row_number = 0;
    for (auto index : GetColumnIndexes()) {
        auto sentinals = GetColumn(index).GetFirstLastRowNumbers();
        first_row_number = min(first_row_number, sentinals.front());
        last_row_number = max(last_row_number, sentinals.back());
    }
    return RowNumberSentinals(first_row_number, last_row_number);
}

bool Frame::HasEntireLineColumn() const {
    const auto &indexes = GetColumnIndexes();
    return find(begin(indexes), end(indexes), ENTIRE_LINE) != end(indexes);
}

size_t Frame::GetColumnCount(bool include_name_value) const {
    auto total_count = size_t{0};
    const auto &indexes = GetColumnIndexes();
    if (!include_name_value) {
        // when not calculating column{row(name, value) - column{row(name, value)}
        // the entire_line column will overlap with the first row
        total_count = size(indexes) - (HasEntireLineColumn() ? 1 : 0);
    } else {
        // in this case we are just worried about the column count
        // if entire_line is the only column then calcuate it's column count
        // otherwise add up the number of row(name, value) pairs
        for (auto index : indexes) {
            if (index == ENTIRE_LINE) {
                if (size(indexes) == 1) {
                    total_count = GetColumn(index).GetColumnCount();
                    break;
                }
            } else {
                total_count += GetColumn(index).GetColumnCount();
            }
        }
    }
    LOG_VERBOSE("column count: {}, include name value: {}", total_count, include_name_value);
    return total_count;
}

const Row &Frame::GetRow(RowId row_id) const {
    const auto &indexes = GetColumnIndexes();
    auto i = find_if(begin(indexes), end(indexes), [&row_id, this](auto index) {
        const auto &column = GetColumn(index);
        return column.ContainsRow(row_id);
    });
    CHECKS(i != end(indexes), "unable to find row id: {}", row_id);
    return GetColumn(*i).GetRow(row_id);
}

Row &Frame::GetRow(RowId row_id) {
    const auto &indexes = GetColumnIndexes();
    auto i = find_if(begin(indexes), end(indexes), [&row_id, this](auto index) {
        const auto &column = GetColumn(index);
        return column.ContainsRow(row_id);
    });
    CHECKS(i != end(indexes), "unable to find row id: {}", row_id);
    return GetColumn(*i).GetRow(row_id);
}

size_t Frame::GetRowHeight(int row_number) const {
    auto height = size_t{0};
    for (auto index : GetColumnIndexes()) {
        const auto &column = GetColumn(index);
        if (column.ContainsRow(row_number)) {
            height = max(height, column.GetRow(row_number).GetHeight());
        }
    }
    return height;
}

Rows Frame::GetRows(int row_number) const {
    auto rows = Rows{};
    for (auto index : GetColumnIndexes()) {
        const auto &column = GetColumn(index);
        if (column.ContainsRow(row_number)) {
            rows.push_back(column.GetRow(row_number));
        }
    }
    return rows;
}

RowIds Frame::GetRowIds(int row_number) const {
    const auto &rows = GetRows(row_number);
    auto row_ids = RowIds{};
    transform(begin(rows), end(rows), back_inserter(row_ids), [](const auto &row) { return row.GetId(); });
    return row_ids;
}

void Frame::Create(OutputTemplate config, const Informations &informations) {
    const auto &items = config.GetColumnItems();
    AddRowsColumns(items, informations);
    AddNamesValues(informations);
    PrepareForOutput(config.GetOutputSettings());
}

Frame Frame::CreateFrame(OutputTemplate config, const Informations &informations) {
    auto frame = Frame{};
    frame.Create(config, informations);
    return frame;
}

string Frame::to_string() const {
    return CreateTable();
}

} // namespace mmotd::results
