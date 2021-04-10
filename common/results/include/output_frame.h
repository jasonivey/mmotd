// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/results/include/output_column.h"
#include "common/results/include/output_column_indexes.h"
#include "common/results/include/output_row.h"
#include "common/results/include/output_row_number_sentinals.h"
#include "common/results/include/output_template.h"
#include "common/results/include/template_column_items.h"

#include <string>
#include <utility>
#include <vector>

namespace mmotd::information {

class Informations;

} // namespace mmotd::information

namespace mmotd::results {

class Frame {
public:
    Frame() = default;
    static Frame CreateFrame(OutputTemplate config, const mmotd::information::Informations &informations);

    std::string to_string() const;

private:
    void Create(OutputTemplate config, const mmotd::information::Informations &informations);

    void AddRowsColumns(mmotd::results::data::TemplateColumnItems items,
                        const mmotd::information::Informations &informations);
    void AddNamesValues(const mmotd::information::Informations &informations);

    void PrepareForOutput(mmotd::results::data::OutputSettings settings);

    std::pair<std::vector<int>, std::vector<RowIds>> GetColumnRowNumbers() const;
    std::string CreateTable() const;

    RowNumberSentinals GetFirstLastRowNumbers() const;
    bool HasEntireLineColumn() const;
    std::size_t GetColumnCount(bool include_name_value) const;
    std::size_t GetRowHeight(int row_number) const;

    Rows GetRows(int row_number) const;
    RowIds GetRowIds(int row_number) const;

    const Row &GetRow(RowId row_id) const;
    Row &GetRow(RowId row_id);

    void RemoveEmptyRows();
    void ReindexRows();
    void CollapseColumnRows();

    void AddRow(data::TemplateColumnItem item, int &increment_row, std::size_t count = 1);
    std::size_t GetRowValueInformationIdCount(const std::string &value,
                                              const information::Informations &informations) const;

    ColumnIndexes GetColumnIndexes() const;

    bool ContainsColumn(ColumnIndex index) const;

    const Column &GetColumn(ColumnIndex index) const;
    Column &GetColumn(ColumnIndex index);

    const Column &GetColumn(RowId row_id) const;
    Column &GetColumn(RowId row_id);

    using ColumnType = std::pair<ColumnIndex, Column>;
    using Columns = std::vector<ColumnType>;

    Columns columns_;
    ColumnIndexes column_indexes_;
    std::string table_type_;
};

} // namespace mmotd::results
