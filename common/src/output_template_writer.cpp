// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/output_template_writer.h"

#include "common/assertion/include/assertion.h"
#include "common/include/algorithm.h"
#include "common/include/config_options.h"
#include "common/include/information.h"
#include "common/include/logging.h"
#include "common/include/output_template.h"
#include "common/include/string_utils.h"
#include "common/include/template_column_items.h"

#include <algorithm>
#include <iterator>
#include <numeric>
#include <optional>
#include <ostream>
#include <regex>
#include <string>
#include <string_view>
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
using mmotd::output_template::OutputTemplate;
using mmotd::output_template::TemplateColumnItem;
using mmotd::output_template::TemplateColumnItems;
namespace sutils = mmotd::string_utils;

namespace {

inline size_t GetUtf8DisplayLength(string input) {
    input = sutils::RemoveAsciiEscapeCodes(input);
    if (auto i = utf8::find_invalid(input); i != string_view::npos) {
        LOG_ERROR("invalid utf8: {}, at offset: {}", input, i);
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

bool StrsReferencesIdNotFound(const vector<string> &strs, const Informations &informations) {
    const auto pattern = regex(R"(%(ID_[_A-Z]+)%)");
    auto i = find_if(begin(strs), end(strs), [&pattern, &informations](auto &str) {
        auto match = smatch{};
        if (!regex_search(str, match, pattern)) {
            return false;
        }
        auto id = mmotd::information::from_information_id_string(match[1].str());
        return id != InformationId::ID_INVALID_INVALID_INFORMATION && !informations.contains(id);
    });
    return i != end(strs);
}

size_t GetInformationReferenceCount(const vector<string> &strs, const Informations &informations) {
    const auto pattern = regex(R"(%(ID_[_A-Z]+)%)");
    for (const auto &str : strs) {
        auto match = smatch{};
        if (!regex_search(str, match, pattern)) {
            continue;
        }
        auto id = mmotd::information::from_information_id_string(match[1].str());
        if (id != InformationId::ID_INVALID_INVALID_INFORMATION && informations.contains(id)) {
            return std::size(informations.at(id));
        }
    }
    return size_t{0};
}

bool ItemReferencesIdNotFound(const TemplateColumnItem &item, const Informations &informations) {
    return StrsReferencesIdNotFound(item.name, informations) || StrsReferencesIdNotFound(item.value, informations);
}

size_t GetInformationReferenceCount(const TemplateColumnItem &item, const Informations &informations) {
    auto name_count = GetInformationReferenceCount(item.name, informations);
    auto value_count = GetInformationReferenceCount(item.value, informations);
    return std::max(name_count, value_count);
}

auto DuplicateRepeatableItems(TemplateColumnItems items, const Informations &informations) -> TemplateColumnItems {
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

auto RemoveOptionalItems(TemplateColumnItems items, const Informations &informations) -> TemplateColumnItems {
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

namespace mmotd::output_template_writer {

auto ReplaceInformationIds(TemplateColumnItems items, const Informations &informations) -> TemplateColumnItems {
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
                auto text_style = mmotd::output_template::color::from_color_string(color);
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

class ColumnData {
public:
    explicit ColumnData(optional<string> data);
    ColumnData() = default;
    ~ColumnData() = default;
    ColumnData(const ColumnData &) = default;
    ColumnData(ColumnData &&) = default;
    ColumnData &operator=(const ColumnData &) = default;
    ColumnData &operator=(ColumnData &&) = default;

    bool HasData() const noexcept { return data_.has_value(); }
    string GetData() const noexcept { return data_.value_or(""s); }
    size_t GetWidth() const noexcept { return width_; }
    void SetWidth(size_t new_width) noexcept { width_ = new_width; }
    bool empty() const noexcept { return !data_.has_value(); }

private:
    optional<string> data_;
    size_t width_ = size_t{0};
};

ColumnData::ColumnData(optional<string> data) : data_(move(data)), width_(GetUtf8DisplayLength(data_.value_or(""s))) {
    PRECONDITIONS(utf8::is_valid(data_.value_or(""s)), "invalid utf8 column data: {}", data_.value_or(""s));
}

class NameValueColumn {
    friend ostream &operator<<(ostream &os, const NameValueColumn &column);

public:
    NameValueColumn(optional<string> name, optional<string> value, size_t indent);
    NameValueColumn() = default;
    ~NameValueColumn() = default;
    NameValueColumn(const NameValueColumn &) = default;
    NameValueColumn(NameValueColumn &&) = default;
    NameValueColumn &operator=(const NameValueColumn &) = default;
    NameValueColumn &operator=(NameValueColumn &&) = default;

    bool empty() const noexcept;
    size_t size() const noexcept;

    pair<size_t, size_t> GetMaxColumnWidths(size_t name_width, size_t value_width) const;
    void SetColumnWidths(size_t name_width, size_t value_width);

private:
    ColumnData name_;
    ColumnData value_;
    size_t indent_ = static_cast<size_t>(output_template::DEFAULT_INDENT_SIZE);
};

NameValueColumn::NameValueColumn(optional<string> name, optional<string> value, size_t indent) :
    name_(move(name)),
    value_(move(value)),
    indent_(indent) {
    PRECONDITIONS(name_.HasData() || value_.HasData(), "empty name/value column");
}

bool NameValueColumn::empty() const noexcept {
    return std::empty(name_) && std::empty(value_);
}

size_t NameValueColumn::size() const noexcept {
    if (std::empty(name_) && std::empty(value_)) {
        // A placeholder column is size=2 because it has a potential of having a name and value
        return size_t{2};
    } else if (std::empty(name_) || std::empty(value_)) {
        // Strictly speaking, only the `name_` field is allowed to be empty, but we'll allow either in this calculation
        return size_t{1};
    } else {
        // In the most common case name and value are both present
        return size_t{2};
    }
}

pair<size_t, size_t> NameValueColumn::GetMaxColumnWidths(size_t name_width, size_t value_width) const {
    name_width = max(name_width, name_.GetWidth());
    value_width = max(value_width, value_.GetWidth());
    return make_pair(name_width, value_width);
}

void NameValueColumn::SetColumnWidths(size_t name_width, size_t value_width) {
    name_.SetWidth(name_width);
    value_.SetWidth(value_width);
}

// os << "[" << std::setfill('*') << std::left << std::setw(name_width) << column.name << "]";
// os << " [" << std::setfill('*') << std::left << std::setw(value_width) << column.value << "]";
ostream &operator<<(ostream &os, const NameValueColumn &column) {
    if (column.indent_ > 0) {
        os << string(column.indent_, ' ');
    }
    if (!std::empty(column.name_) || std::empty(column)) {
        auto name_width = column.name_.GetWidth() + sutils::GetAsciiEscapeCodesSize(column.name_.GetData());
        auto name = column.name_.HasData() && !std::empty(column.name_.GetData()) ? column.name_.GetData() :
                                                                                    string(name_width, ' ');
        os << std::setfill(' ') << std::left << std::setw(static_cast<int>(name_width)) << name;
    }
    // if there is no value string or this is a spacer, add in the space between the name and the value
    if (!std::empty(column.name_) || std::empty(column)) {
        os << " "s;
    }
    if (!std::empty(column.value_) || std::empty(column)) {
        auto value_width = column.value_.GetWidth() + sutils::GetAsciiEscapeCodesSize(column.value_.GetData());
        auto value = column.value_.HasData() && !std::empty(column.value_.GetData()) ? column.value_.GetData() :
                                                                                       string(value_width, ' ');
        os << std::left << std::setw(static_cast<int>(value_width)) << value;
    }
    return os;
}

class EmptyRows {
public:
    EmptyRows(size_t count) : count_(count) {}
    EmptyRows() = default;
    ~EmptyRows() = default;
    EmptyRows(const EmptyRows &) = default;
    EmptyRows(EmptyRows &&) = default;
    EmptyRows &operator=(const EmptyRows &) = default;
    EmptyRows &operator=(EmptyRows &&) = default;

    auto count() const noexcept { return count_; }
    void set_count(size_t count) noexcept { count_ = count; }

private:
    size_t count_ = size_t{0};
};

ostream &operator<<(ostream &os, const EmptyRows &row) {
    return os << string(row.count(), '\n');
}

class OutputRow {
    friend ostream &operator<<(ostream &os, const OutputRow &row);

public:
    OutputRow(size_t column_count) : columns_(column_count, NameValueColumn{}) {}
    OutputRow() = default;
    ~OutputRow() = default;
    OutputRow(const OutputRow &) = default;
    OutputRow(OutputRow &&) = default;
    OutputRow &operator=(const OutputRow &) = default;
    OutputRow &operator=(OutputRow &&) = default;

    bool IsEntireLine() const noexcept;
    void SetEntireLine(bool entire_line) noexcept { is_entire_line_ = entire_line; }

    void SetPrependEmptyRows(size_t new_count) {
        prepend_empty_rows_.set_count(std::clamp(new_count, size_t{0}, size_t{100}));
    }
    void SetAppendEmptyRows(size_t new_count) {
        append_empty_rows_.set_count(std::clamp(new_count, size_t{1}, size_t{100}));
    }

    size_t GetTotalColumnCount() const noexcept;
    size_t GetColumnCount() const noexcept;
    void ReserveColumnCount(size_t count) noexcept;
    bool IsColumnEmpty(size_t column_index) const noexcept { return std::empty(columns_.at(column_index)); }
    bool empty() const noexcept { return std::empty(columns_); }

    void SetColumnData(size_t column_index, const TemplateColumnItem &item, optional<string> name, string value);

    void GetMaxColumnWidths(vector<size_t> &widths) const;
    void SetColumnWidths(const vector<size_t> &widths);

private:
    EmptyRows prepend_empty_rows_;
    vector<NameValueColumn> columns_;
    EmptyRows append_empty_rows_{1};
    bool is_entire_line_ = false;
};

bool OutputRow::IsEntireLine() const noexcept {
    return is_entire_line_;
}

size_t OutputRow::GetTotalColumnCount() const noexcept {
    return std::accumulate(begin(columns_), end(columns_), size_t{0}, [](size_t count, const auto &column) {
        return count + std::size(column);
    });
}

size_t OutputRow::GetColumnCount() const noexcept {
    return std::size(columns_);
}

void OutputRow::ReserveColumnCount(size_t count) noexcept {
    for (auto i = GetColumnCount(); i < count; ++i) {
        columns_.emplace_back();
    }
}

void OutputRow::SetColumnData(size_t column_index,
                              const TemplateColumnItem &item,
                              optional<string> name,
                              string value) {
    if (!name.has_value()) {
        CHECKS(item.IsEntireLine(), "column with only a value must be entire line");
        CHECKS(GetColumnCount() == size_t{1} && std::empty(columns_.front()), "unable to overwrite existing data");
        columns_.front() = NameValueColumn{nullopt, value, static_cast<size_t>(item.indent_size)};
    } else {
        CHECKS(column_index < GetColumnCount(), "invalid column index");
        CHECKS(std::empty(columns_[column_index]), "unable to overwrite existing data");
        columns_[column_index] = NameValueColumn{name, value, static_cast<size_t>(item.indent_size)};
    }
    SetEntireLine(item.IsEntireLine());
}

void OutputRow::GetMaxColumnWidths(vector<size_t> &widths) const {
    PRECONDITIONS((GetColumnCount() * 2) == std::size(widths), "widths list must be equal to column count");
    auto index = size_t{0};
    for_each(begin(columns_), end(columns_), [&widths, &index](const auto &column) {
        CHECKS(index + 1 < std::size(widths), "index into the widths list is out of bounds");
        tie(widths[index], widths[index + 1]) = column.GetMaxColumnWidths(widths[index], widths[index + 1]);
        index += 2;
    });
}

void OutputRow::SetColumnWidths(const vector<size_t> &widths) {
    PRECONDITIONS((GetColumnCount() * 2) == std::size(widths), "widths list must be equal to column count");
    auto index = size_t{0};
    for_each(begin(columns_), end(columns_), [&widths, &index](auto &column) {
        CHECKS(index + 1 < std::size(widths), "index into the widths list is out of bounds");
        column.SetColumnWidths(widths[index], widths[index + 1]);
        index += 2;
    });
}

ostream &operator<<(ostream &os, const OutputRow &row) {
    os << row.prepend_empty_rows_;
    for (const auto &column : row.columns_) {
        os << column;
    }
    return os << row.append_empty_rows_;
}

class OutputRows {
    friend ostream &operator<<(ostream &os, const OutputRows &rows);

public:
    OutputRows(vector<int> indexes) : column_indexes(move(indexes)) {}

    bool AddItem(const TemplateColumnItem &item);
    void SetColumnWidths();

private:
    size_t GetColumnIndex(int column) const;
    size_t GetColumnCount(int column) const noexcept;
    OutputRow &GetCurrentRow(int column, size_t column_count);

    static bool HasUnresolvedInformationIds(const vector<string> &strs);
    static bool HasUnresolvedInformationId(const string &str);
    static bool IsValidValueItem(const TemplateColumnItem &item);
    static bool IsValidNameItem(const TemplateColumnItem &item);
    static bool IsValidNameValueItem(const TemplateColumnItem &item);

    void AddValueItem(const TemplateColumnItem &item);
    void AddNameItem(const TemplateColumnItem &item);
    void AddNameValueItem(const TemplateColumnItem &item);

    OutputRow &AddRow(const TemplateColumnItem &item, optional<string> name, string value);

    bool IsColorOutputEnabled() const noexcept;

    vector<OutputRow> rows;
    vector<int> column_indexes;
};

size_t OutputRows::GetColumnIndex(int column) const {
    static constexpr size_t ENTIRE_LINE_INDEX = 0;
    if (column == output_template::ENTIRE_LINE) {
        return ENTIRE_LINE_INDEX;
    } else {
        auto col_indexes = column_indexes;
        col_indexes.erase(remove(begin(col_indexes), end(col_indexes), output_template::ENTIRE_LINE), end(col_indexes));
        auto i = find(begin(col_indexes), end(col_indexes), column);
        if (i == end(col_indexes)) {
            THROW_INVALID_ARGUMENT("column index not found: {}", column);
        }
        return static_cast<size_t>(distance(begin(col_indexes), i));
    }
}

size_t OutputRows::GetColumnCount(int column) const noexcept {
    auto column_count = size_t{0};
    if (column == output_template::ENTIRE_LINE) {
        column_count = size_t{1};
    } else {
        column_count = count_if(begin(column_indexes), end(column_indexes), [](int index) {
            return index != output_template::ENTIRE_LINE;
        });
    }
    return column_count;
}

OutputRow &OutputRows::GetCurrentRow(int column, size_t column_count) {
    OutputRow *row = nullptr;
    if (std::empty(rows)) {
        rows.emplace_back();
        row = &rows.back();
    } else if (column == output_template::ENTIRE_LINE) {
        if (rows.back().IsEntireLine() && std::empty(rows.back())) {
            row = &rows.back();
        } else {
            rows.emplace_back();
            row = &rows.back();
        }
    } else {
        auto index = GetColumnIndex(column);
        auto last_row = rend(rows);
        for (auto i = rbegin(rows); i != rend(rows); ++i) {
            if (index >= (*i).GetColumnCount() || !(*i).IsColumnEmpty(index)) {
                // ensure that the current row is large enough and empty enough to hold the new column
                break;
            } else if (i != rbegin(rows) && i - 1 != last_row) {
                // ensure that we have a contigous set of rows
                break;
            } else {
                last_row = i;
            }
        }
        if (last_row != rend(rows)) {
            row = &(*last_row);
        } else {
            rows.emplace_back();
            row = &rows.back();
        }
    }
    CHECKS(row != nullptr, "unable to find current row and did not allocate a new one");
    row->ReserveColumnCount(column_count);
    return *row;
}

OutputRow &OutputRows::AddRow(const TemplateColumnItem &item, optional<string> name, string value) {
    auto &row = GetCurrentRow(item.column, GetColumnCount(item.column));
    row.SetColumnData(GetColumnIndex(item.column), item, name, value);
    return row;
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
        auto &row = AddRow(item, nullopt, value_str);
        if (i == size_t{0}) {
            row.SetPrependEmptyRows(static_cast<size_t>(item.prepend_newlines));
        }
        if (i + 1 == size(item.value)) {
            row.SetAppendEmptyRows(static_cast<size_t>(item.append_newlines));
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
        auto &row = AddRow(item, nullopt, name_str);
        if (i == size_t{0}) {
            row.SetPrependEmptyRows(static_cast<size_t>(item.prepend_newlines));
        }
        if (i + 1 == size(item.value)) {
            row.SetAppendEmptyRows(static_cast<size_t>(item.append_newlines));
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
        auto &row = AddRow(item, name_str, value_str);
        if (i == size_t{0}) {
            row.SetPrependEmptyRows(static_cast<size_t>(item.prepend_newlines));
        }
        if (i + 1 == size(item.value)) {
            row.SetAppendEmptyRows(static_cast<size_t>(item.append_newlines));
        }
    }
}

inline bool OutputRows::HasUnresolvedInformationIds(const vector<string> &strs) {
    auto i = find_if(begin(strs), end(strs), [](const auto &str) { return HasUnresolvedInformationId(str); });
    return i != end(strs);
}

inline bool OutputRows::HasUnresolvedInformationId(const string &str) {
    const auto pattern = regex(R"(%(ID_[_A-Z]+)%)");
    return regex_search(str, pattern);
}

inline bool OutputRows::IsValidValueItem(const TemplateColumnItem &item) {
    return empty(item.name) && !empty(item.value) && item.IsEntireLine() && !HasUnresolvedInformationIds(item.value);
}

inline bool OutputRows::IsValidNameItem(const TemplateColumnItem &item) {
    return !empty(item.name) && empty(item.value) && item.IsEntireLine() && !HasUnresolvedInformationIds(item.name);
}

inline bool OutputRows::IsValidNameValueItem(const TemplateColumnItem &item) {
    return !empty(item.name) && !empty(item.value) && !HasUnresolvedInformationIds(item.name) &&
           !HasUnresolvedInformationIds(item.value);
}

bool OutputRows::AddItem(const TemplateColumnItem &item) {
    if (HasUnresolvedInformationIds(item.name) || HasUnresolvedInformationIds(item.value)) {
        return false;
    }
    if (OutputRows::IsValidValueItem(item)) {
        // Add entire line column for each item.value
        AddValueItem(item);
        return true;
    } else if (OutputRows::IsValidNameItem(item)) {
        // Add entire line column for each item.name
        AddNameItem(item);
        return true;
    } else if (OutputRows::IsValidNameValueItem(item)) {
        // Add column for each item.name and item.value
        AddNameValueItem(item);
        return true;
    }
    return false;
}

void OutputRows::SetColumnWidths() {
    auto first_name_column_width = size_t{0};
    auto begin_range = begin(rows);
    while (begin_range != end(rows)) {
        auto i = adjacent_find(begin_range, end(rows), [](const auto &row_a, const auto &row_b) {
            return row_a.GetTotalColumnCount() != row_b.GetTotalColumnCount();
        });
        auto end_range = i == end(rows) ? end(rows) : i + 1;
        auto column_count = begin_range->GetColumnCount();
        auto widths = vector<size_t>(column_count * 2, size_t{0});
        if (!std::empty(widths)) {
            widths.front() = first_name_column_width;
        }
        for_each(begin_range, end_range, [&widths](const auto &row) { row.GetMaxColumnWidths(widths); });
        for_each(begin_range, end_range, [&widths](auto &row) { row.SetColumnWidths(widths); });
        first_name_column_width = std::empty(widths) ? size_t{0} : widths.front();
        begin_range = end_range;
    }
}

bool OutputRows::IsColorOutputEnabled() const noexcept {
    static const auto color_output_enabled = ConfigOptions::Instance().GetBoolean("core.output_color"sv, true);
    return color_output_enabled;
}

ostream &operator<<(ostream &os, const OutputRows &rows) {
    for_each(begin(rows.rows), end(rows.rows), [&os](auto &row) { os << row; });
    return os;
}

OutputTemplateWriter::OutputTemplateWriter(vector<int> column_indexes,
                                           TemplateColumnItems items,
                                           Informations informations) :
    column_indexes_{std::move(column_indexes)},
    items_{std::move(items)},
    informations_{std::move(informations)} {}

ostream &operator<<(ostream &os, const OutputTemplateWriter &output) {
    auto items = ReplaceInformationIds(output.items_, output.informations_);
    items = ReplaceEmbeddedColorCodes(items);
    auto rows = OutputRows{output.column_indexes_};

    for (auto i = begin(items); i != end(items); ++i) {
        auto &item = *i;
        if (!rows.AddItem(item) && i != begin(items)) {
            // carry backward any prepended and appended newlines
            auto &prev_item = *(i - 1);
            prev_item.prepend_newlines += item.prepend_newlines;
            prev_item.append_newlines += (item.append_newlines - 1);
        }
    }

    rows.SetColumnWidths();
    return os << rows;
}

string to_string(const OutputTemplateWriter &writer) {
    auto os = ostringstream{};
    os << writer;
    return os.str();
}

} // namespace mmotd::output_template_writer
