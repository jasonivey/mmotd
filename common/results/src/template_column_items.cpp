// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/algorithm.h"
#include "common/results/include/template_column_items.h"
#include "common/results/include/template_string.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <iterator>
#include <optional>
#include <regex>
#include <system_error>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <fmt/color.h>
#include <nlohmann/json.hpp>
#include <plog/Log.h>

using fmt::format;
using nlohmann::json;
using namespace std;
using mmotd::results::data::ENTIRE_LINE, mmotd::results::data::ENTIRE_LINE_REPR;

namespace {

static constexpr const auto TerminalColors =
    array<const char *, 8>{"black", "red", "green", "yellow", "blue", "magenta", "cyan", "white"};
static constexpr const auto TerminalColorIndexes = array<fmt::terminal_color, 16>{fmt::terminal_color::black,
                                                                                  fmt::terminal_color::red,
                                                                                  fmt::terminal_color::green,
                                                                                  fmt::terminal_color::yellow,
                                                                                  fmt::terminal_color::blue,
                                                                                  fmt::terminal_color::magenta,
                                                                                  fmt::terminal_color::cyan,
                                                                                  fmt::terminal_color::white,
                                                                                  fmt::terminal_color::bright_black,
                                                                                  fmt::terminal_color::bright_red,
                                                                                  fmt::terminal_color::bright_green,
                                                                                  fmt::terminal_color::bright_yellow,
                                                                                  fmt::terminal_color::bright_blue,
                                                                                  fmt::terminal_color::bright_magenta,
                                                                                  fmt::terminal_color::bright_cyan,
                                                                                  fmt::terminal_color::bright_white};

vector<string> ConvertTerminalColors() {
    return vector<string>{begin(TerminalColors), end(TerminalColors)};
}

optional<uint32_t> FromString(string_view str, int base = 10) {
    auto result = uint32_t{0};
    if (auto [ptr, ec] = from_chars(begin(str), end(str), result, base); ec == std::errc{}) {
        return make_optional(result);
    } else {
        PLOG_ERROR << format(FMT_STRING("unable to convert {} into an integer, {}"),
                             str,
                             make_error_code(ec).message());
        return nullopt;
    }
}

optional<fmt::text_style> GetHexColorValue(string value) {
    const auto pattern = regex(R"(^hex\(\s*([0-9A-Fa-f]{2}|0)\s*([0-9A-Fa-f]{2})?\s*([0-9A-Fa-f]{2})?\s*\)$)",
                               std::regex_constants::ECMAScript | std::regex_constants::icase);
    auto matches = smatch{};
    if (regex_match(value, matches, pattern)) {
        auto hex_str = matches.str(1);
        if (matches.size() > 2) {
            hex_str += matches.str(2);
        }
        if (matches.size() > 3) {
            hex_str += matches.str(3);
        }
        auto hex_value = FromString(string_view(hex_str), 16);
        if (hex_value) {
            return make_optional(fmt::fg(fmt::rgb(*hex_value)));
        }
    }
    return nullopt;
}

optional<fmt::text_style> GetRgbColorValue(string value) {
    const auto pattern = regex(R"(^rgb\(\s*(\d{1,3})\s*,\s*(\d{1,3})\s*,\s*(\d{1,3})\s*\)$)",
                               std::regex_constants::ECMAScript | std::regex_constants::icase);
    auto matches = smatch{};
    if (regex_match(value, matches, pattern)) {
        auto red = FromString(string_view(matches.str(1)));
        auto green = FromString(string_view(matches.str(2)));
        auto blue = FromString(string_view(matches.str(3)));
        if (!red || !green || !blue || *red > 0xFF || *green > 0xFF || *blue > 0xFF) {
            PLOG_ERROR << format(FMT_STRING("invalid rgb value red={}, blue={}, green={} (all values should be 0-255)"),
                                 *red,
                                 *blue,
                                 *green);
            return nullopt;
        }
        auto rgb_value = (*red << 16) | (*green << 8) | *blue;
        return make_optional(fmt::fg(fmt::rgb(rgb_value)));
    }
    return nullopt;
}

optional<fmt::text_style> GetTerminalPlainColor(string value, bool bright) {
    const auto colors = string{"black|red|green|yellow|blue|magenta|cyan|white"};
    const auto color_regex = regex(colors, regex_constants::ECMAScript | regex_constants::icase);
    auto match = smatch{};
    if (!regex_search(value, match, color_regex)) {
        PLOG_ERROR << format(FMT_STRING("no terminal color was specified within {} (valid colors are: {})"),
                             value,
                             colors);
        return nullopt;
    }
    auto color_str = match.str();
    auto i = find_if(begin(TerminalColors), end(TerminalColors), [color_str](const auto &color_name) {
        return boost::iequals(color_name, color_str);
    });
    if (i == end(TerminalColors)) {
        auto color_list = format(FMT_STRING("[{}]"), boost::join(ConvertTerminalColors(), string{", "}));
        PLOG_ERROR << format(FMT_STRING("terminal color regex matched to {} but unable to find color in {}"),
                             color_str,
                             color_list);
        return nullopt;
    }
    auto offset = distance(begin(TerminalColors), i);
    auto color = fmt::terminal_color::black;
    if (bright) {
        color = *(TerminalColorIndexes.begin() + std::size(TerminalColors) + offset);
    } else {
        color = *(TerminalColorIndexes.begin() + offset);
    }
    PLOG_VERBOSE << format(FMT_STRING("found a terminal color, offset={}, color code={} within value={}"),
                           offset,
                           static_cast<uint8_t>(color),
                           value);
    return make_optional(fmt::fg(color));
}

optional<tuple<bool, bool, bool, bool, bool>> GetTerminalColorEmphasis(string value) {
    auto bold = false, italic = false, underline = false, strikethrough = false, bright = false;
    if (boost::icontains(value, "bold_") || boost::icontains(value, "_bold")) {
        bold = true;
        PLOG_VERBOSE << format(FMT_STRING("found bold specified in terminal color {}"), value);
    }
    if (boost::icontains(value, "italic_") || boost::icontains(value, "_italic")) {
        italic = true;
        PLOG_VERBOSE << format(FMT_STRING("found italic specified in terminal color {}"), value);
    }
    if (boost::icontains(value, "underline_") || boost::icontains(value, "_underline")) {
        underline = true;
        PLOG_VERBOSE << format(FMT_STRING("found underline specified in terminal color {}"), value);
    }
    if (boost::icontains(value, "strikethrough_") || boost::icontains(value, "_strikethrough")) {
        strikethrough = true;
        PLOG_VERBOSE << format(FMT_STRING("found strikethrough specified in terminal color {}"), value);
    }
    if (boost::icontains(value, "bright_") || boost::icontains(value, "_bright")) {
        bright = true;
        PLOG_VERBOSE << format(FMT_STRING("found bright specified in terminal color {}"), value);
    }
    return make_optional(make_tuple(bold, italic, underline, strikethrough, bright));
}

optional<fmt::text_style> GetTerminalColorValue(string value) {
    auto emphasis_holder = GetTerminalColorEmphasis(value);
    if (!emphasis_holder) {
        return nullopt;
    }
    auto [bold, italic, underline, strikethrough, bright] = *emphasis_holder;
    auto plain_color_holder = GetTerminalPlainColor(value, bright);
    if (!plain_color_holder) {
        return nullopt;
    }
    auto txt_style = *plain_color_holder;
    if (bold) {
        txt_style |= fmt::emphasis::bold;
    }
    if (italic) {
        txt_style |= fmt::emphasis::italic;
    }
    if (underline) {
        txt_style |= fmt::emphasis::underline;
    }
    if (strikethrough) {
        txt_style |= fmt::emphasis::strikethrough;
    }
    return make_optional(txt_style);
}

optional<fmt::text_style> GetColorValue(string value) {
    if (boost::istarts_with(value, "hex(")) {
        return GetHexColorValue(value);
    } else if (boost::istarts_with(value, "rgb(")) {
        return GetRgbColorValue(value);
    } else {
        return GetTerminalColorValue(value);
    }
}

inline int column_from_string(const string &column_str) {
    using namespace boost;
    if (iequals(string_view(data(column_str)), ENTIRE_LINE_REPR)) {
        return ENTIRE_LINE;
    } else if (auto column_holder = FromString(string_view(data(column_str))); column_holder) {
        return static_cast<int>(*column_holder);
    } else {
        auto msg = string{
            "Input template json contains invalid columns.  Columns is an array of strings.  Each array element must be a valid integer or 'ENTIRE_LINE'"};
        MMOTD_THROW_OUT_OF_RANGE(msg);
        return -1;
    }
}

inline string column_to_string(int column_value) {
    using namespace mmotd::results::data;
    return column_value == ENTIRE_LINE ? string{ENTIRE_LINE_REPR} : to_string(column_value);
}

} // namespace

namespace mmotd::results::color {

string to_string(fmt::text_style txt_style) {
    if (!txt_style.has_foreground()) {
        return string{};
    }
    if (txt_style.get_foreground().is_rgb) {
        return format(FMT_STRING("hex({:06X})"), txt_style.get_foreground().value.rgb_color);
    }
    auto terminal_color = string{};
    if (txt_style.has_emphasis()) {
        auto emp = txt_style.get_emphasis();
        if ((static_cast<uint8_t>(emp) & static_cast<uint8_t>(fmt::emphasis::bold)) != 0) {
            terminal_color += "bold_";
        }
        if ((static_cast<uint8_t>(emp) & static_cast<uint8_t>(fmt::emphasis::italic)) != 0) {
            terminal_color += "italic_";
        }
        if ((static_cast<uint8_t>(emp) & static_cast<uint8_t>(fmt::emphasis::underline)) != 0) {
            terminal_color += "underline_";
        }
        if ((static_cast<uint8_t>(emp) & static_cast<uint8_t>(fmt::emphasis::strikethrough)) != 0) {
            terminal_color += "strikethrough_";
        }
    }
    auto terminal_value = static_cast<uint8_t>(txt_style.get_foreground().value.term_color);
    if (terminal_value >= static_cast<uint8_t>(fmt::terminal_color::bright_black)) {
        terminal_color += "bright_";
    }
    auto index = terminal_value % 10;
    return terminal_color + *(begin(TerminalColors) + index);
}

fmt::text_style from_color_string(string input) {
    auto text_style_holder = ::GetColorValue(input);
    return text_style_holder.has_value() ? text_style_holder.value() : fmt::text_style{};
}

} // namespace mmotd::results::color

namespace mmotd::results::data {

string TemplateItemSettings::to_string() const {
    return format(FMT_STRING(R"(indent_size: {},
row_index: {},
repeatable_index: {},
column: {},
prepend_newlines: {},
append_newlines: {},
is_repeatable: {},
is_optional: {},
name: {},
name_color: {},
value: {},
value_color: {})"),
                  indent_size,
                  row_index,
                  repeatable_index,
                  column,
                  prepend_newlines,
                  append_newlines,
                  is_repeatable,
                  is_optional,
                  fmt::join(name, ", "),
                  color::to_string(name_color),
                  fmt::join(value, ", "),
                  color::to_string(value_color));
}

bool TemplateItemSettings::validate(const TemplateConfig &default_config) {
    auto i = find(begin(default_config.columns), end(default_config.columns), column);
    if (i == end(default_config.columns)) {
        PLOG_ERROR << format(FMT_STRING("column item at row index={} and column={} but config.columns olny specify {}"),
                             row_index,
                             column_to_string(column),
                             default_config.columns_to_string());
        return false;
    }
    if (empty(name) && empty(value)) {
        PLOG_ERROR << format(FMT_STRING("column item at row index={} does not have a name or a value"), row_index);
        return false;
    }
    return true;
}

void TemplateItemSettings::from_json(const json &root, const TemplateItemSettings *default_settings) {
    if (root.contains("indent_size")) {
        root.at("indent_size").get_to(indent_size);
    } else if (default_settings != nullptr) {
        indent_size = default_settings->indent_size;
    }
    if (root.contains("row_index")) {
        // no default taken from default_settings
        root.at("row_index").get_to(row_index);
    } else if (default_settings != nullptr) {
        // default_settings == nullptr is only true when creating default_settings
        PLOG_WARNING << "item missing row_index property (must have it for ordering purposes)";
    }
    if (root.contains("repeatable_index")) {
        root.at("repeatable_index").get_to(repeatable_index);
    } else if (default_settings != nullptr) {
        repeatable_index = default_settings->repeatable_index;
    }
    if (root.contains("column")) {
        column = column_from_string(root.at("column"));
    } else if (default_settings != nullptr) {
        column = default_settings->column;
    }
    if (root.contains("prepend_newlines")) {
        root.at("prepend_newlines").get_to(prepend_newlines);
    } else if (default_settings != nullptr) {
        prepend_newlines = default_settings->prepend_newlines;
    }
    if (root.contains("append_newlines")) {
        root.at("append_newlines").get_to(append_newlines);
    } else if (default_settings != nullptr) {
        append_newlines = default_settings->append_newlines;
    }
    if (root.contains("is_repeatable")) {
        root.at("is_repeatable").get_to(is_repeatable);
    } else if (default_settings != nullptr) {
        is_repeatable = default_settings->is_repeatable;
    }
    if (root.contains("is_optional")) {
        root.at("is_optional").get_to(is_optional);
    } else if (default_settings != nullptr) {
        is_optional = default_settings->is_optional;
    }
    if (root.contains("name")) {
        // no default taken from default_settings
        root.at("name").get_to(name);
    } else if (default_settings != nullptr) {
        // default_settings == nullptr is only true when creating default_settings
        PLOG_WARNING << "item missing name property (should leave an empty [] name for completeness)";
    }
    if (root.contains("name_color")) {
        name_color = color::from_color_string(root.at("name_color"));
    } else if (default_settings != nullptr) {
        name_color = default_settings->name_color;
    }
    if (root.contains("value")) {
        // no default taken from default_settings
        root.at("value").get_to(value);
    } else if (default_settings != nullptr) {
        // default_settings == nullptr is only true when creating default_settings
        PLOG_WARNING << "item missing value property (should leave an empty [] value for completeness)";
    }
    if (root.contains("value_color")) {
        value_color = color::from_color_string(root.at("value_color"));
    } else if (default_settings != nullptr) {
        value_color = default_settings->value_color;
    }
}

void TemplateItemSettings::to_json(json &root) const {
    root = json{{"indent_size", indent_size},
                // {"row_index", row_index},
                // {"repeatable_index", repeatable_index},
                {"column", column_to_string(column)},
                {"prepend_newlines", prepend_newlines},
                {"append_newlines", append_newlines},
                {"is_repeatable", is_repeatable},
                {"is_optional", is_optional},
                {"name", name},
                {"name_color", color::to_string(name_color)},
                {"value", value},
                {"value_color", color::to_string(value_color)}};
}

void to_json(json &root, const TemplateItemSettings &settings) {
    settings.to_json(root);
}

void from_json(const json &root, TemplateItemSettings &settings) {
    settings.from_json(root, nullptr);
}

std::string OutputSettings::to_string() const {
    auto collapse_column_rows_str = format(FMT_STRING("collapse_column_rows = {}\n"), collapse_column_rows);
    auto table_type_str = format(FMT_STRING("table_type = {}"), table_type);
    return collapse_column_rows_str + table_type_str;
}

void OutputSettings::from_json(const json &root) {
    if (root.contains("collapse_column_rows")) {
        root.at("collapse_column_rows").get_to(collapse_column_rows);
    }
    if (root.contains("table_type")) {
        root.at("table_type").get_to(table_type);
    }
}

void OutputSettings::to_json(json &root) const {
    root = json{{"collapse_column_rows", collapse_column_rows}, {"table_type", table_type}};
}

void from_json(const json &root, OutputSettings &settings) {
    settings.from_json(root);
}

void to_json(json &root, const OutputSettings &settings) {
    settings.to_json(root);
}

string TemplateConfig::to_string() const {
    auto columns_str = format(FMT_STRING("columns = {}\n"), columns_to_string());
    auto output_settings_str = format(FMT_STRING("output_settings = {}\n"), output_settings_to_string());
    auto settings_str = format(FMT_STRING("default_settings = [\n{}]"), default_settings_to_string());
    return columns_str + output_settings_str + settings_str;
}

string TemplateConfig::output_settings_to_string() const {
    return output_settings.to_string();
}

string TemplateConfig::default_settings_to_string() const {
    return default_settings.to_string();
}

string TemplateConfig::columns_to_string() const {
    using mmotd::algorithms::join;
    auto columns_str = join(columns, ", ", [](int value) { return column_to_string(value); });
    return format(FMT_STRING("[{}]"), columns_str);
}

void TemplateConfig::columns_from_json(const json &root) {
    columns.clear();
    if (!root.contains("columns")) {
        auto msg =
            "No columns were specified in input json.  Defaulting to one column containing one row each (ENTIRE_LINE).";
        PLOG_WARNING << msg;
        columns.push_back(ENTIRE_LINE);
        return;
    }
    for (auto column : root["columns"]) {
        columns.push_back(column_from_string(column));
    }
}

void TemplateConfig::default_settings_from_json(const json &root) {
    if (root.contains("default_settings")) {
        root.at("default_settings").get_to(default_settings);
    }
}

void TemplateConfig::output_settings_from_json(const json &root) {
    if (root.contains("output_settings")) {
        root.at("output_settings").get_to(output_settings);
    }
}

void TemplateConfig::from_json(const json &root) {
    columns_from_json(root);
    default_settings_from_json(root);
    output_settings_from_json(root);
}

void from_json(const json &root, TemplateConfig &template_config) {
    template_config.from_json(root);
}

void TemplateConfig::to_json(json &root) const {
    auto columns_strs = vector<string>{};
    for_each(begin(columns), end(columns), [&columns_strs](int value) {
        columns_strs.push_back(column_to_string(value));
    });
    root =
        json{{"columns", columns_strs}, {"default_settings", default_settings}, {"output_settings", output_settings}};
}

void to_json(json &root, const TemplateConfig &template_config) {
    template_config.to_json(root);
}

string to_string(const TemplateColumnItems &items) {
    return mmotd::algorithms::join(items, "\n", [](const auto &item) {
        return format(FMT_STRING("item: [\n{}]\n"), item.to_string());
    });
}

} // namespace mmotd::results::data
