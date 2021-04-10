// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "common/include/big_five_macros.h"
#include "common/results/include/output_position_index.h"
#include "common/results/include/output_row.h"
#include "common/results/include/output_row_number_sentinals.h"
#include "common/results/include/template_column_items.h"

namespace mmotd::information {

class Informations;

} // namespace mmotd::information

namespace mmotd::results {

class Column {
public:
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(Column);

    void AddRow(mmotd::results::data::TemplateColumnItem item, int &increment_row, std::size_t count = 1);
    void AddNamesValues(const mmotd::information::Informations &informations, PositionIndex position_index);
    bool BalanceNameValueSize();
    void ApplyTemplateSettings(PositionIndex position_index);

    PositionIndex GetPositionIndex() const noexcept { return position_index_; }
    std::size_t GetColumnCount() const;

    const Rows &GetRows() const { return rows_; }
    Rows &GetRows() { return rows_; }

    bool ContainsRow(RowId row_id) const;
    bool ContainsRow(int row_number) const;

    bool HasPreviousRow(RowId row_id) const;
    const Row &GetPreviousRow(RowId row_id) const;
    Row &GetPreviousRow(RowId row_id);

    const Row &GetRow(RowId row_id) const;
    Row &GetRow(RowId row_id);

    Row &GetRow(int row_number);
    const Row &GetRow(int row_number) const;

    void CollapseRows();
    RowNumberSentinals GetFirstLastRowNumbers() const;

private:
    void SetPositionIndex(PositionIndex position_index);

    PositionIndex position_index_;
    Rows rows_;
};

} // namespace mmotd::results
