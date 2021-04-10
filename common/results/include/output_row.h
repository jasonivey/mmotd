// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "common/assertion/include/assertion.h"
#include "common/include/big_five_macros.h"
#include "common/results/include/output_position_index.h"
#include "common/results/include/template_column_items.h"

#include <cstdint>
#include <string>
#include <vector>

#include <boost/uuid/uuid.hpp>

namespace mmotd::information {

class Informations;

} // namespace mmotd::information

namespace mmotd::results {

using RowId = boost::uuids::uuid;
using RowIds = std::vector<RowId>;

class Row;
using Rows = std::vector<Row>;

class Row {
public:
    explicit Row(mmotd::results::data::TemplateColumnItem item);
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(Row);

    int GetRowNumber() const noexcept;
    int GetColumnNumber() const noexcept;
    PositionIndex GetColumnPosition() const noexcept;
    std::string GetColumnNumberStr() const noexcept;
    std::size_t GetHeight() const;
    std::size_t GetColumnCount() const;

    void SetPositionIndex(PositionIndex position_index);
    void SetNameValue(const mmotd::information::Informations &informations);
    bool BalanceNameValueSize();
    std::size_t UpdateRowNumber(int row_number);

    std::string item_to_string() const { return item_.to_string(); }

    bool HasName(std::size_t index) const;
    bool HasValue(std::size_t index) const;

    std::string GetName(std::size_t index) const;
    std::string GetValue(std::size_t index) const;

    int GetPrependNewlineCount() const noexcept { return item_.prepend_newlines; }
    int GetIndentCount() const noexcept { return item_.indent_size; }
    int GetAppendNewlineCount() const noexcept { return item_.append_newlines - 1; }

    RowId GetId() const noexcept { return id_; }

private:
    void AddName(std::string name) { names_.emplace_back(name); }
    void AddValue(std::string value) { values_.emplace_back(value); }

    static RowId GenerateRowId();

    std::vector<std::string> names_;
    std::vector<std::string> values_;
    PositionIndex position_index_;
    mmotd::results::data::TemplateColumnItem item_;
    RowId id_ = GenerateRowId();
};

} // namespace mmotd::results
