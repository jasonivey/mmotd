// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/results/include/output_row.h"
#include "common/results/include/output_table.h"

#include <memory>
#include <regex>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <fort.hpp>
#include <plog/Log.h>

using fmt::format;
using namespace std;

namespace {

// auto Table::TableImpl::GetTableStyle(string style_str) -> std::remove_cvref_t<int>decltype(FT_PLAIN_STYLE) {
decltype(auto) GetTableStyle(string style_str) {
    using namespace boost;
    if (iends_with(style_str, "BASIC2_STYLE")) {
        return FT_BASIC2_STYLE;
    } else if (iends_with(style_str, "SIMPLE_STYLE")) {
        return FT_SIMPLE_STYLE;
    } else if (iends_with(style_str, "PLAIN_STYLE")) {
        return FT_PLAIN_STYLE;
    } else if (iends_with(style_str, "DOT_STYLE")) {
        return FT_DOT_STYLE;
    } else if (iends_with(style_str, "EMPTY_STYLE")) {
        return FT_EMPTY_STYLE;
    } else if (iends_with(style_str, "EMPTY2_STYLE")) {
        return FT_EMPTY2_STYLE;
    } else if (iends_with(style_str, "SOLID_STYLE")) {
        return FT_SOLID_STYLE;
    } else if (iends_with(style_str, "SOLID_ROUND_STYLE")) {
        return FT_SOLID_ROUND_STYLE;
    } else if (iends_with(style_str, "NICE_STYLE")) {
        return FT_NICE_STYLE;
    } else if (iends_with(style_str, "DOUBLE_STYLE")) {
        return FT_DOUBLE_STYLE;
    } else if (iends_with(style_str, "DOUBLE2_STYLE")) {
        return FT_DOUBLE2_STYLE;
    } else if (iends_with(style_str, "BOLD_STYLE")) {
        return FT_BOLD_STYLE;
    } else if (iends_with(style_str, "BOLD2_STYLE")) {
        return FT_BOLD2_STYLE;
    } else if (iends_with(style_str, "FRAME_STYLE")) {
        return FT_FRAME_STYLE;
    } else if (iends_with(style_str, "BASIC_STYLE")) {
        return FT_BASIC_STYLE;
    } else {
        return FT_PLAIN_STYLE;
    }
}

string RemoveColorCodes(string input) {
    // all color codes start with 0x1b, "[", other stuff in the middle, ending with "m"
    const auto color_code_regex = regex(R"(\x1b\[[^m]+m)", std::regex::ECMAScript);
    auto output = regex_replace(input, color_code_regex, "");
    return output;
}

int GetStringWidth(const void *vbeg, const void *vend, size_t *width) {
    static auto width_cache = vector<pair<string, size_t>>{};
    if (vbeg == nullptr || vend == nullptr) {
        PLOG_ERROR << "unable to get string width when pointer to begin or end is null";
        return 1;
    }

    auto input = string(reinterpret_cast<const char *>(vbeg), reinterpret_cast<const char *>(vend));
    auto i = find_if(begin(width_cache), end(width_cache), [&input](const auto &cache_item) {
        const auto &[cache_str, _] = cache_item;
        return input == cache_str;
    });

    auto input_width = size_t{0};
    if (i != end(width_cache)) {
        const auto &[_, cached_width] = *i;
        input_width = cached_width;
    } else {
        auto stripped_str = RemoveColorCodes(input);
        const char *buffer_ptr = data(stripped_str);
        auto state = mbstate_t{};
        input_width = mbsrtowcs(nullptr, &buffer_ptr, 0, &state);
        width_cache.push_back(make_pair(input, input_width));
    }

    if (width != nullptr) {
        *width = input_width;
        return 0;
    } else {
        PLOG_ERROR << "unable to return string width output width is null";
        return 1;
    }
}

} // namespace

namespace mmotd::results {

class Table::TableImpl {
public:
    TableImpl(string table_style, size_t column_count);

    string to_string() const;

    void WriteRow(const Row &row);

private:
    string GetRowPrefix(const Row &row) const;
    string GetRowSuffix(const Row &row, size_t line_index) const;

    void WriteCell(string text, size_t row_number, size_t col_number, size_t span, bool is_entire_line);
    void WriteNameValue(const Row &row, size_t line_index);
    void WriteName(const Row &row, size_t line_index);
    void WriteValue(const Row &row, size_t line_index, bool bump_column);

    //using TableType = fort::utf8_table;
    using TableType = fort::char_table;
    TableType &GetTable() { return table_; }
    const TableType &GetTable() const { return table_; }
    size_t GetColumnCount() const { return column_count_; }

    TableType table_;
    size_t column_count_;
};

Table::TableImpl::TableImpl(string table_style, size_t column_count) : table_(), column_count_(column_count) {
    PLOG_VERBOSE << format("setting border style: {}", table_style);
    GetTable().set_border_style(GetTableStyle(table_style));
    PLOG_VERBOSE << "setting ft_set_u8strwid_func to custom: GetStringWidth";
    ft_set_u8strwid_func(&GetStringWidth);
    PLOG_VERBOSE << "setting adding strategy: replace";
    GetTable().set_adding_strategy(fort::add_strategy::replace);
    PLOG_VERBOSE << format("setting right margin: {}", GetColumnCount());
    GetTable().set_right_margin(static_cast<unsigned>(GetColumnCount()));
}

string Table::TableImpl::to_string() const {
    return GetTable().to_string();
}

void Table::TableImpl::WriteRow(const Row &row) {
    auto already_written_name = false;
    auto row_line_count = row.GetHeight();
    for (auto i = size_t{0}; i != row_line_count; ++i) {
        PLOG_VERBOSE << format("writing [{}][{}] {} column",
                               row.GetRowNumber(),
                               row.GetColumnNumberStr(),
                               row.GetColumnPosition().to_string());
        if (row.HasName(i) && row.HasValue(i)) {
            WriteNameValue(row, i);
            already_written_name = true;
        } else if (row.HasName(i)) {
            WriteName(row, i);
            already_written_name = true;
        } else {
            WriteValue(row, i, already_written_name);
        }
    }
    if (row.GetColumnPosition().IsEndOfLine()) {
        GetTable() << fort::endr;
    }
}

string Table::TableImpl::GetRowPrefix(const Row &row) const {
    auto column_position = row.GetColumnPosition();
    auto newline_count = static_cast<size_t>(row.GetPrependNewlineCount());
    auto indent_count = static_cast<size_t>(row.GetIndentCount());
    if (column_position.IsStartOfLine()) {
        return string(newline_count, '\n') + string(indent_count, ' ');
    }
    return string{};
}

string Table::TableImpl::GetRowSuffix(const Row &row, size_t line_index) const {
    auto column_position = row.GetColumnPosition();
    auto newline_count = static_cast<size_t>(row.GetAppendNewlineCount());
    if (column_position.IsEndOfLine() && line_index + 1 == row.GetHeight()) {
        return string(newline_count, '\n');
    }
    return string{};
}

void Table::TableImpl::WriteCell(string text,
                                 size_t row_number,
                                 size_t col_number,
                                 size_t cell_span,
                                 bool use_cell_span) {
    PLOG_VERBOSE << format("setting right margin: {}", GetColumnCount());
    GetTable().set_right_margin(static_cast<unsigned>(GetColumnCount()));
    auto debug_str = "writing cell, row: {}, column: {}, cell span: {}, text: '{}'";
    auto cell_span_str = use_cell_span ? ::to_string(cell_span) : string{"none"};
    PLOG_VERBOSE << format(debug_str, row_number, col_number, cell_span_str, text);
    GetTable().set_cur_cell(row_number, col_number);
    if (use_cell_span) {
        GetTable().cur_cell().set_cell_span(cell_span);
    }
    GetTable() << text;
}

void Table::TableImpl::WriteNameValue(const Row &row, size_t line_index) {
    auto row_number = static_cast<size_t>(row.GetRowNumber()) + line_index;
    auto column_position = row.GetColumnPosition();
    auto column_number = column_position.GetIndex() * row.GetColumnCount();
    auto cell_span = GetColumnCount() - 1;
    auto text = format("{}{}", GetRowPrefix(row), row.GetName(line_index));
    WriteCell(text, row_number, column_number, size_t{0}, false);
    text = format("{}{}", row.GetValue(line_index), GetRowSuffix(row, line_index));
    WriteCell(text, row_number, column_number + 1, cell_span, column_position.IsEntireLine());
}

void Table::TableImpl::WriteName(const Row &row, size_t line_index) {
    auto row_number = static_cast<size_t>(row.GetRowNumber()) + line_index;
    auto column_position = row.GetColumnPosition();
    auto column_number = column_position.GetIndex() * row.GetColumnCount();
    auto cell_span = GetColumnCount() - 1;
    auto text = format("{}{}{}", GetRowPrefix(row), row.GetName(line_index), GetRowSuffix(row, line_index));
    WriteCell(text, row_number, column_number, cell_span, column_position.IsEntireLine());
}

void Table::TableImpl::WriteValue(const Row &row, size_t line_index, bool bump_column) {
    auto row_number = static_cast<size_t>(row.GetRowNumber()) + line_index;
    auto column_position = row.GetColumnPosition();
    auto column_number = column_position.GetIndex() * row.GetColumnCount();
    column_number += bump_column ? 1 : 0;
    auto cell_span = bump_column ? GetColumnCount() - 1 : GetColumnCount();
    auto text = format("{}{}{}", GetRowPrefix(row), row.GetValue(line_index), GetRowSuffix(row, line_index));
    WriteCell(text, row_number, column_number, cell_span, column_position.IsEntireLine());
}

Table::Table(string table_style_str, size_t column_count) :
    table_(make_unique<TableImpl>(table_style_str, column_count)) {
}

Table::~Table() {
}

void Table::WriteRow(const Row &row) {
    table_->WriteRow(row);
}

string Table::to_string() const {
    return table_->to_string();
}

} // namespace mmotd::results
