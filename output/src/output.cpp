// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/algorithm.h"
#include "common/include/config_options.h"
#include "common/include/informations.h"
#include "common/include/logging.h"
#include "common/include/string_utils.h"
#include "common/results/include/output_template.h"
#include "common/results/include/template_column_items.h"
#include "output/include/output.h"

#include <algorithm>
#include <clocale>
#include <cstddef>
#include <cstdlib>
#include <cwchar>
#include <iostream>
#include <iterator>
#include <locale>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <fmt/color.h>
#include <fmt/format.h>
#include <utf8.h>
#include <utf8/cpp17.h>

using namespace std;
using namespace std::string_literals;
using mmotd::core::ConfigOptions;
using mmotd::information::Information;
using mmotd::information::InformationId;
using mmotd::information::Informations;
using mmotd::results::OutputTemplate;
using mmotd::results::data::TemplateColumnItem;
using mmotd::results::data::TemplateColumnItems;
namespace sutils = mmotd::string_utils;

namespace {

inline size_t GetUtf8DisplayLength(string input) {
    input = sutils::RemoveAsciiEscapeCodes(input);
    if (auto i = utf8::find_invalid(input); i != string_view::npos) {
        LOG_ERROR("invalid utf8: {} at offset {}", input, i);
        input = utf8::replace_invalid(input);
    }
    return utf8::distance(input.begin(), input.end());
}

struct ColorSpecification {
    std::vector<std::string_view> specs;
    std::string_view text;
    std::size_t offset = std::string_view::npos;
    std::size_t length = std::string_view::npos;
};

struct ColorSpecificationData {
    std::string_view prefix;
    ColorSpecification color_specification;
};

using ColorSpecifications = std::vector<ColorSpecificationData>;
// FIX_TODO: jasoni the definition of Informations should be this way:
using AllInformations = unordered_map<InformationId, vector<Information>>;

auto CompactInformationIds(const Informations &informations) -> AllInformations {
    auto all_informations = AllInformations{};
    for (const auto &information : informations) {
        auto id = information.GetId();
        if (all_informations.contains(id)) {
            all_informations[id].push_back(information);
        } else {
            all_informations[id] = {information};
        }
    }
    return all_informations;
}

bool StrsReferencesIdNotFound(const vector<string> &strs, const AllInformations &informations) {
    const auto pattern = regex(R"(%(ID_[_A-Z]+)%)");
    auto i = find_if(begin(strs), end(strs), [&pattern, &informations](auto &str) {
        auto match = smatch{};
        if (!regex_search(str, match, pattern)) {
            return false;
        }
        auto id = mmotd::information::from_string(match[1].str());
        return id != InformationId::ID_INVALID_INVALID_INFORMATION && !informations.contains(id);
    });
    return i != end(strs);
}

size_t GetInformationReferenceCount(const vector<string> &strs, const AllInformations &informations) {
    const auto pattern = regex(R"(%(ID_[_A-Z]+)%)");
    for (const auto &str : strs) {
        auto match = smatch{};
        if (!regex_search(str, match, pattern)) {
            continue;
        }
        auto id = mmotd::information::from_string(match[1].str());
        if (id != InformationId::ID_INVALID_INVALID_INFORMATION && informations.contains(id)) {
            return informations.at(id).size();
        }
    }
    return size_t{0};
}

bool ItemReferencesIdNotFound(const TemplateColumnItem &item, const AllInformations &informations) {
    return StrsReferencesIdNotFound(item.name, informations) || StrsReferencesIdNotFound(item.value, informations);
}

size_t GetInformationReferenceCount(const TemplateColumnItem &item, const AllInformations &informations) {
    auto name_count = GetInformationReferenceCount(item.name, informations);
    auto value_count = GetInformationReferenceCount(item.value, informations);
    return std::max(name_count, value_count);
}

auto DuplicateRepeatableItems(TemplateColumnItems items, const AllInformations &informations) -> TemplateColumnItems {
    auto result = TemplateColumnItems{};
    for (const auto &item : items) {
        if (item.is_repeatable) {
            auto count = GetInformationReferenceCount(item, informations);
            for (auto i = size_t{0}; i < count; ++i) {
                result.push_back(item);
                result.back().repeatable_index = static_cast<int>(i);
            }
        } else {
            result.push_back(item);
        }
    }
    return result;
}

auto RemoveOptionalItems(TemplateColumnItems items, const AllInformations &informations) -> TemplateColumnItems {
    auto result = TemplateColumnItems{};
    for (const auto &item : items) {
        if (item.is_optional && ItemReferencesIdNotFound(item, informations)) {
            continue;
        }
        result.push_back(item);
    }
    return result;
}

} // namespace

namespace mmotd::output {

auto ReplaceInformationIds(TemplateColumnItems items, const Informations &infos) -> TemplateColumnItems {
    auto informations = CompactInformationIds(infos);
    items = DuplicateRepeatableItems(items, informations);
    items = RemoveOptionalItems(items, informations);
    for (const auto &[id, multiple_infos] : informations) {
        for (auto i = begin(multiple_infos); i != end(multiple_infos); ++i) {
            auto index = static_cast<int>(std::distance(begin(multiple_infos), i));
            const auto &information = *i;
            const auto id_str = fmt::format(FMT_STRING("%{}%"), information.GetPlainIdStr());
            const auto &info_value = information.GetValue();
            for (auto &item : items) {
                if (item.repeatable_index != index) {
                    // this is not a true test but will do for 90% of the cases
                    continue;
                }
                for (auto &name_str : item.name) {
                    boost::replace_all(name_str, id_str, info_value);
                }
                for (auto &value_str : item.value) {
                    boost::replace_all(value_str, id_str, info_value);
                }
            }
        }
    }
    return items;
}

// Input -> GOOD: %color:bold_bright_green%
//           BAD: %color:bold_bright_green
//                                        ^ missing closing '%'
//          GOOD: %color:bold_bright_green:bold_bright_red%
//                                        ^ using colon ':' to seperate color definitions
//          GOOD: %color:bold_bright_green%%color:bold_bright_red%
//                                        ^ includes a closing '%' for the first color definition
//           BAD: %color:bold_bright_green%color:bold_bright_red%
//                                        ^ only includes one '%'
auto FindColorSpecifications(string_view input) -> ColorSpecifications {
    if (empty(input)) {
        return {};
    }
    //%color:bold_bright_green%
    static constexpr auto COLOR_SPECIFICATION_PREFIX = string_view{"%color:"};
    static constexpr auto COLOR_SPECIFICATION_SUFFIX = string_view{"%"};
    auto color_specs = ColorSpecifications{};
    auto pos = 0ull;
    while (pos < size(input)) {
        auto begin_index = input.find(COLOR_SPECIFICATION_PREFIX, pos);
        if (begin_index == string_view::npos) {
            break;
        }
        auto prefix = pos == 0ull && begin_index != 0ull ? string_view(data(input), begin_index) : string_view{};
        if (!empty(color_specs)) {
            auto &last_text = color_specs.back().color_specification.text;
            last_text = string_view(data(last_text), begin_index - pos);
        }
        auto end_index = input.find(COLOR_SPECIFICATION_SUFFIX, begin_index + size(COLOR_SPECIFICATION_PREFIX));
        // check if ending % is not found OR if ending % is found but its the start of the next specification
        if (end_index == string_view::npos || input.substr(end_index).starts_with(COLOR_SPECIFICATION_PREFIX)) {
            LOG_ERROR("no ending color specification '%' found in '{}' at offset: {}", input, begin_index);
            color_specs.clear();
            break;
        }
        // cut off the "%color:" & "%" begin/end delimiters from the result
        pos = begin_index + size(COLOR_SPECIFICATION_PREFIX);
        auto length = end_index - pos;
        auto current_specs = input.substr(pos, length);
        auto split_specs = mmotd::algorithms::split(current_specs, ":");
        auto color_specification =
            ColorSpecification{move(split_specs),
                               string_view(begin(input) + end_index + size(COLOR_SPECIFICATION_SUFFIX)),
                               begin_index,
                               end_index + size(COLOR_SPECIFICATION_SUFFIX)};
        auto color_spec_data = ColorSpecificationData{prefix, move(color_specification)};
        color_specs.push_back(move(color_spec_data));
        pos = end_index + size(COLOR_SPECIFICATION_SUFFIX);
    }
    if (empty(color_specs)) {
        auto color_spec_data = ColorSpecificationData{input, ColorSpecification{}};
        return ColorSpecifications{color_spec_data};
    }
    return color_specs;
}

auto ReplaceColorSpecifications(const ColorSpecifications &color_specs, bool test) -> string {
    if (empty(color_specs)) {
        return string{};
    }
    static const auto color_output = ConfigOptions::Instance().GetBoolean("core.output_color"sv, true);
    auto output = string(color_specs.front().prefix);
    for (const auto &color_spec_data : color_specs) {
        const auto &colors = color_spec_data.color_specification.specs;
        if (test) {
            for_each(begin(colors), end(colors), [&output](const auto &color) {
                output += format(FMT_STRING("[color:{}]"), color);
            });
            output += color_spec_data.color_specification.text;
        } else if (!color_output) {
            output += color_spec_data.color_specification.text;
        } else {
            if (empty(color_spec_data.color_specification.text)) {
                continue;
            }
            const auto &input_text = color_spec_data.color_specification.text;
            for_each(begin(colors), end(colors), [&output, input_text](const auto &color) {
                auto text_style = mmotd::results::color::from_color_string(color);
                output += format(text_style, FMT_STRING("{}"), input_text);
            });
        }
    }
    return output;
}

auto FindAndReplaceColorSpecifications(string_view input, bool test) -> string {
    auto color_specs = FindColorSpecifications(input);
    return ReplaceColorSpecifications(color_specs, test);
}

auto ReplaceEmbeddedColorCodes(TemplateColumnItems items) -> TemplateColumnItems {
    for (auto &item : items) {
        for (auto &name_str : item.name) {
            name_str = FindAndReplaceColorSpecifications(name_str);
        }
        for (auto &value_str : item.value) {
            value_str = FindAndReplaceColorSpecifications(value_str);
        }
    }
    return items;
}

struct ColumnData {
    const TemplateColumnItem *item = nullptr;
    optional<string> name;
    optional<size_t> name_width;
    string value;
    size_t value_width = 0ull;

    size_t GetColumnCount() const noexcept { return 1ull + (std::empty(name.value_or(""s)) ? 0ull : 1ull); }
    size_t size() const noexcept { return GetColumnCount(); }
    bool empty() const noexcept { return !name && std::empty(value); }
};

#if 0
struct ColumnNameValue {
    const TemplateColumnItem *item = nullptr;
    string name;
    string value;
};

struct ColumnValue {
    const TemplateColumnItem *item = nullptr;
    string value;
    size_t value_width = 0ull;
};

using ColumnData = variant<ColumnNameValue, ColumnValue>;
#endif

struct EmptyRows {
    size_t count = 0ull;
    friend ostream &operator<<(ostream &os, const EmptyRows &row) { return os << string(row.count, '\n'); }
};

struct OutputRow {
    static constexpr size_t ENTIRE_LINE_INDEX = 0;
    EmptyRows prepend_empty_rows;
    vector<ColumnData> columns;
    EmptyRows append_empty_rows{1};
    bool is_entire_line = false;

    bool IsEntireLine() const noexcept;
    void SetEntireLine(bool entire_line) noexcept { is_entire_line = entire_line; }

    size_t GetColumnCount() const noexcept;
    size_t size() const noexcept { return GetColumnCount(); }
    bool empty() const noexcept { return size() == size_t{0}; }

    void AddColumn(const TemplateColumnItem &item, string value, size_t column_count);
    void AddColumn(const TemplateColumnItem &item, string name, string value, size_t column_count);
    friend ostream &operator<<(ostream &os, const OutputRow &row);
};

bool OutputRow::IsEntireLine() const noexcept {
    return is_entire_line;
}

size_t OutputRow::GetColumnCount() const noexcept {
    auto column_count = size_t{0};
    for (auto &column : columns) {
        column_count += column.GetColumnCount();
    }
    return column_count;
}

void OutputRow::AddColumn(const TemplateColumnItem &item, string value, size_t column_count) {
    PRECONDITIONS(column_count > 0, "column count must be greater than zero");
    PRECONDITIONS(item.IsEntireLine(), "column with only a value must be entire line");
    PRECONDITIONS(std::empty(columns), "unable to add entire line to row with existing columns");
    auto column = ColumnData{};
    column.item = &item;
    column.value = move(value);
    column.value_width = GetUtf8DisplayLength(value);
    SetEntireLine(true);
    columns.push_back(move(column));
    for (auto i = size_t{1}; i < column_count; ++i) {
        columns.push_back(ColumnData{});
    }
}

void OutputRow::AddColumn(const TemplateColumnItem &item, string name, string value, size_t column_count) {
    PRECONDITIONS(column_count > 0, "column count must be greater than zero");
    auto column = ColumnData{};
    column.item = &item;
    column.name = move(name);
    column.name_width = GetUtf8DisplayLength(name);
    column.value = move(value);
    column.value_width = GetUtf8DisplayLength(value);
    columns.push_back(move(column));
    SetEntireLine(item.IsEntireLine());
    for (auto i = size_t{1}; i < column_count; ++i) {
        columns.push_back(ColumnData{});
    }
}

ostream &operator<<(ostream &os, const OutputRow &row) {
    os << row.prepend_empty_rows;
    for (const auto &column : row.columns) {
        const auto &item = *column.item;
        CHECKS(utf8::is_valid(column.value), "invalid utf8: {}", column.value);
        CHECKS(utf8::is_valid(column.name.value_or(""s)), "invalid utf8: {}", column.name.value_or(""s));
        if (column.name.has_value()) {
            const auto &name = column.name.value();
            auto indent = string(static_cast<size_t>(item.indent_size), ' ');
            os << indent;
            auto name_width = static_cast<int>(column.name_width.value_or(0) + sutils::GetAsciiEscapeCodesSize(name));
            // os << "[" << std::setfill('*') << std::left << std::setw(name_width) << column.name << "]";
            os << std::left << std::setw(name_width) << name;
        }
        auto value_width = static_cast<int>(column.value_width + sutils::GetAsciiEscapeCodesSize(column.value));
        // os << " [" << std::setfill('*') << std::left << std::setw(value_width) << column.value << "]";
        os << " "s;
        os << std::left << std::setw(value_width) << column.value;
    }
#if 0
        else if (holds_alternative<ColumnValue>(column)) {
            auto &column_value = get<ColumnValue>(column);
            CHECKS(utf8::is_valid(column_value.value), "invalid utf8: {}", column_value.value);
            const auto &item = *column_value.item;
            auto indent = string(static_cast<size_t>(item.indent_size), ' ');
            os << indent;
            auto value_width =
                static_cast<int>(column_value.value_width + sutils::GetAsciiEscapeCodesSize(column_value.value));
            // os << "[" << std::setfill('*') << std::left << std::setw(value_width) << column_value.value << "]";
            os << std::left << std::setw(value_width) << column_value.value;
        }
    }
#endif
    return os << row.append_empty_rows;
}

class OutputRows {
    friend ostream &operator<<(ostream &os, const OutputRows &rows);

public:
    OutputRows(vector<int> indexes) : column_indexes(move(indexes)) {}

    void AddItem(const TemplateColumnItem &item);
    void AddColumnWidths();

private:
    OutputRow &GetCurrentRow(int column);
    size_t GetColumnIndex(int column) const;
    static size_t GetColumnCount(vector<int> column_indexes, int column);

    void AddValueItem(const TemplateColumnItem &item);
    void AddNameItem(const TemplateColumnItem &item);
    void AddNameValueItem(const TemplateColumnItem &item);
    void AddRow(const TemplateColumnItem &item, string value);
    void AddRow(const TemplateColumnItem &item, string name, string value);

    void
    SetColumnWidths(vector<OutputRow>::iterator begin, vector<OutputRow>::iterator end, const vector<size_t> &widths);
    bool IsColorOutputEnabled() const noexcept;

    vector<OutputRow> rows;
    vector<int> column_indexes;
};

size_t OutputRows::GetColumnIndex(int column) const {
    if (column == results::data::ENTIRE_LINE) {
        return OutputRow::ENTIRE_LINE_INDEX;
    } else {
        for (size_t i = 0; i < std::size(column_indexes); ++i) {
            if (column_indexes[i] == column) {
                return i;
            }
        }
        ALWAYS_FAIL("column index not found: {}", column);
    }
}

OutputRow &OutputRows::GetCurrentRow(int column) {
    if (rows.empty()) {
        rows.emplace_back();
        return rows.back();
    } else if (column == results::data::ENTIRE_LINE) {
        return rows.back();
    } else {
        auto index = GetColumnIndex(column);
        if (index < rows.back().size() && rows.back().columns[index].empty()) {
            return rows.back();
        } else {
            rows.emplace_back();
            return rows.back();
        }
    }
}

size_t OutputRows::GetColumnCount(vector<int> column_indexes, int column) {
    if (column == results::data::ENTIRE_LINE) {
        return 1;
    } else {
        return count_if(begin(column_indexes), end(column_indexes), [](int index) {
            return index != results::data::ENTIRE_LINE;
        });
    }
}

void OutputRows::AddRow(const TemplateColumnItem &item, string value) {
    auto &row = GetCurrentRow(item.column);
    row.AddColumn(item, value, GetColumnCount(column_indexes, item.column));
}

void OutputRows::AddRow(const TemplateColumnItem &item, string name, string value) {
    auto &row = GetCurrentRow(item.column);
    row.AddColumn(item, name, value, GetColumnCount(column_indexes, item.column));
}

void OutputRows::AddValueItem(const TemplateColumnItem &item) {
    auto value_str = string{};
    for (auto i = size_t{0}; i != size(item.value); ++i) {
        if (IsColorOutputEnabled()) {
            auto value_style = i < size(item.value_color) ? item.value_color[i] : fmt::text_style{};
            value_str = fmt::format(value_style, FMT_STRING("{}"), item.value[i]);
        } else {
            value_str = item.value[i];
        }
        AddRow(item, value_str);
        if (i == size_t{0}) {
            rows.back().prepend_empty_rows = EmptyRows{static_cast<size_t>(item.prepend_newlines)};
        }
        if (i + 1 == size(item.value)) {
            rows.back().append_empty_rows = EmptyRows{static_cast<size_t>(item.append_newlines)};
        }
    }
}

void OutputRows::AddNameItem(const TemplateColumnItem &item) {
    auto name_str = string{};
    for (auto i = size_t{0}; i != size(item.name); ++i) {
        if (IsColorOutputEnabled()) {
            auto name_style = i < size(item.name_color) ? item.name_color[i] : fmt::text_style{};
            name_str = fmt::format(name_style, FMT_STRING("{}"), item.name[i]);
        } else {
            name_str = item.name[i];
        }
        AddRow(item, name_str);
        if (i == size_t{0}) {
            rows.back().prepend_empty_rows = EmptyRows{static_cast<size_t>(item.prepend_newlines)};
        }
        if (i + 1 == size(item.value)) {
            rows.back().append_empty_rows = EmptyRows{static_cast<size_t>(item.append_newlines)};
        }
    }
}

void OutputRows::AddNameValueItem(const TemplateColumnItem &item) {
    auto length = std::max(size(item.name), size(item.value));
    auto name_str = string{};
    auto value_str = string{};
    for (auto i = 0ull; i < length; ++i) {
        if (IsColorOutputEnabled()) {
            auto name_style = i < size(item.name_color) ? item.name_color[i] : fmt::text_style{};
            name_str = fmt::format(name_style, FMT_STRING("{}"), i < size(item.name) ? item.name[i] : string{});
            auto value_style = i < size(item.value_color) ? item.value_color[i] : fmt::text_style{};
            value_str = fmt::format(value_style, FMT_STRING("{}"), i < size(item.value) ? item.value[i] : string{});
        } else {
            name_str = i < size(item.name) ? item.name[i] : string{};
            value_str = i < size(item.value) ? item.value[i] : string{};
        }
        AddRow(item, name_str, value_str);
        if (i == size_t{0}) {
            rows.back().prepend_empty_rows = EmptyRows{static_cast<size_t>(item.prepend_newlines)};
        }
        if (i + 1 == size(item.value)) {
            rows.back().append_empty_rows = EmptyRows{static_cast<size_t>(item.append_newlines)};
        }
    }
}

void OutputRows::AddItem(const TemplateColumnItem &item) {
    if (empty(item.name) && empty(item.value)) {
        return;
    } else if (empty(item.name) && !empty(item.value) && item.IsEntireLine()) {
        // Add entire line column for each item.value
        AddValueItem(item);
    } else if (!empty(item.name) && empty(item.value) && item.IsEntireLine()) {
        // Add entire line column for each item.name
        AddNameItem(item);
    } else {
        // Add column for each item.name and item.value
        AddNameValueItem(item);
    }
}

void OutputRows::AddColumnWidths() {
    auto prev_column_count = size_t{0};
    auto start_row_range = begin(rows);
    auto widths = vector<size_t>{};
    for (auto i = begin(rows); i != end(rows); ++i) {
        const auto &row = *i;
        if (i == begin(rows)) {
            widths = vector<size_t>(row.GetColumnCount(), size_t{0});
        }
        if (i != begin(rows) && row.GetColumnCount() != prev_column_count) {
            SetColumnWidths(start_row_range, i, widths);
            start_row_range = i;
            widths = vector<size_t>(row.GetColumnCount(), size_t{0});
        }
        auto column_index = size_t{0};
        for (auto &column : row.columns) {
            if (column.name_width.has_value()) {
                widths[column_index] = std::max(widths[column_index], *column.name_width);
                ++column_index;
            }
            widths[column_index] = std::max(widths[column_index], column.value_width);
            ++column_index;
#if 0
            if (holds_alternative<ColumnNameValue>(column)) {
                auto &column_name_value = get<ColumnNameValue>(column);
                widths[column_index] = std::max(widths[column_index], column_name_value.name_width);
                ++column_index;
                widths[column_index] = std::max(widths[column_index], column_name_value.value_width);
                ++column_index;
            } else if (holds_alternative<ColumnValue>(column)) {
                auto &column_value = get<ColumnValue>(column);
                widths[column_index] = std::max(widths[column_index], column_value.value_width);
                ++column_index;
            }
#endif
        }
        prev_column_count = row.GetColumnCount();
    }
    SetColumnWidths(start_row_range, end(rows), widths);
}

void OutputRows::SetColumnWidths(vector<OutputRow>::iterator begin,
                                 vector<OutputRow>::iterator end,
                                 const vector<size_t> &widths) {
    while (begin != end) {
        auto &row = *begin;
        for (auto &column : row.columns) {
            auto widths_index = size_t{0};
            if (column.name_width.has_value()) {
                *column.name_width = widths_index < size(widths) ? widths[widths_index++] : *column.name_width;
            }
            column.value_width = widths_index < size(widths) ? widths[widths_index++] : column.value_width;
#if 0
            if (holds_alternative<ColumnNameValue>(column)) {
                auto &column_name_value = get<ColumnNameValue>(column);
                column_name_value.name_width =
                    widths_index < size(widths) ? widths[widths_index++] : column_name_value.name_width;
                column_name_value.value_width =
                    widths_index < size(widths) ? widths[widths_index++] : column_name_value.value_width;
            } else if (holds_alternative<ColumnValue>(column)) {
                auto &column_value = get<ColumnValue>(column);
                column_value.value_width =
                    widths_index < size(widths) ? widths[widths_index++] : column_value.value_width;
            }
#endif
        }
        ++begin;
    }
}

bool OutputRows::IsColorOutputEnabled() const noexcept {
    static const auto color_output_enabled = ConfigOptions::Instance().GetBoolean("core.output_color"sv, true);
    return color_output_enabled;
}

#if 0
string OutputRows::to_string() const {
    auto output = string{};
    for (auto &row : rows) {
        output += row.to_string();
    }
    return output;
}
#endif

ostream &operator<<(ostream &os, const OutputRows &rows) {
    for (auto &row : rows.rows) {
        os << row;
    }
    return os;
}

string Output::to_string(const OutputTemplate &output_template, const Informations &informations) const {
    auto column_indexes = output_template.GetColumns();
    auto items = ReplaceInformationIds(output_template.GetColumnItems(), informations);
    items = ReplaceEmbeddedColorCodes(items);
    auto rows = OutputRows{column_indexes};
    for_each(begin(items), end(items), [&rows](const auto &item) { rows.AddItem(item); });
    rows.AddColumnWidths();
    cout << rows << endl;
    return string{};
}

} // namespace mmotd::output
