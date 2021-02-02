// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/algorithm.h"
#include "common/include/tty_template_data.h"
#include "common/include/tty_template_string.h"

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
        PLOG_ERROR << format("unable to convert {} into an integer, {}", str, make_error_code(ec).message());
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
            PLOG_ERROR << format("invalid rgb value red={}, blue={}, green={} (all values should be 0-255)",
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
    // const auto pattern = regex(
    //     R"(^((bold_)?)((italic_)?)((underline_)?)((strikethrough_)?)((bright_)?)(black|red|green|yellow|blue|magenta|cyan|white)$)",
    //     std::regex_constants::ECMAScript | std::regex_constants::icase);
    const auto colors = string{"black|red|green|yellow|blue|magenta|cyan|white"};
    const auto color_regex = regex(colors, regex_constants::ECMAScript | regex_constants::icase);
    auto match = smatch{};
    if (!regex_search(value, match, color_regex)) {
        PLOG_ERROR << format("no plain terminal color was specified within {} (i.e. {})", value, colors);
        return nullopt;
    }
    auto color_str = match.str();
    auto i = find_if(begin(TerminalColors), end(TerminalColors), [color_str](const auto &color_name) {
        return boost::iequals(color_name, color_str);
    });
    if (i == end(TerminalColors)) {
        auto color_list = format("[{}]", boost::join(ConvertTerminalColors(), string{", "}));
        PLOG_ERROR << format("terminal color regex matched to {} but unable to find color in {}",
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
    PLOG_VERBOSE << format("found a terminal color, offset={}, color code={} within value={}",
                           offset,
                           static_cast<uint8_t>(color),
                           value);
    return make_optional(fmt::fg(color));
}

optional<tuple<bool, bool, bool, bool, bool>> GetTerminalColorEmphasis(string value) {
    auto bold = false, italic = false, underline = false, strikethrough = false, bright = false;
    if (boost::icontains(value, "bold_") || boost::icontains(value, "_bold")) {
        bold = true;
        PLOG_VERBOSE << format("found bold specified in terminal color {}", value);
    }
    if (boost::icontains(value, "italic_") || boost::icontains(value, "_italic")) {
        italic = true;
        PLOG_VERBOSE << format("found italic specified in terminal color {}", value);
    }
    if (boost::icontains(value, "underline_") || boost::icontains(value, "_underline")) {
        underline = true;
        PLOG_VERBOSE << format("found underline specified in terminal color {}", value);
    }
    if (boost::icontains(value, "strikethrough_") || boost::icontains(value, "_strikethrough")) {
        strikethrough = true;
        PLOG_VERBOSE << format("found strikethrough specified in terminal color {}", value);
    }
    if (boost::icontains(value, "bright_") || boost::icontains(value, "_bright")) {
        bright = true;
        PLOG_VERBOSE << format("found bright specified in terminal color {}", value);
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
        // PLOG_VERBOSE << format("adding bold to color={} for value={}", static_cast<uint8_t>(color), value);
    }
    if (italic) {
        txt_style |= fmt::emphasis::italic;
        // PLOG_VERBOSE << format("adding italic to color={} for value={}", static_cast<uint8_t>(color), value);
    }
    if (underline) {
        txt_style |= fmt::emphasis::underline;
        // PLOG_VERBOSE << format("adding underline to color={} for value={}", static_cast<uint8_t>(color), value);
    }
    if (strikethrough) {
        txt_style |= fmt::emphasis::strikethrough;
        // PLOG_VERBOSE << format("adding strikethrough to color={} for value={}", static_cast<uint8_t>(color), value);
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

} // namespace

namespace mmotd::tty_template::color {

string to_string(fmt::text_style txt_style) {
    if (!txt_style.has_foreground()) {
        return string{};
    }
    if (txt_style.get_foreground().is_rgb) {
        return format("hex({:06X})", txt_style.get_foreground().value.rgb_color);
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

} // namespace mmotd::tty_template::color

namespace mmotd::tty_template::data {

string TemplateItemSettings::to_string() const {
    return format("indent_size: {}\n", indent_size) + format("row_index: {}\n", row_index) +
           format("column: {}\n", column) + format("prepend_newlines: {}\n", prepend_newlines) +
           format("append_newlines: {}\n", append_newlines) + format("is_repeatable: {}\n", is_repeatable) +
           format("is_optional: {}\n", is_optional) + format("name_width: {}\n", name_width) +
           format("name: [{}]\n", boost::join(name, ", ")) +
           format("name_color: {}\n", mmotd::tty_template::color::to_string(name_color)) +
           format("value: [{}]\n", boost::join(value, ", ")) +
           format("value_color: {}", mmotd::tty_template::color::to_string(value_color));
}

bool TemplateItemSettings::validate(const TemplateConfig &default_config) {
    auto i = find(begin(default_config.columns), end(default_config.columns), column);
    if (i == end(default_config.columns)) {
        PLOG_ERROR << format("column item at row index={} and column={} but config.columns olny specify {}",
                             row_index,
                             column,
                             default_config.columns_to_string());
        return false;
    }
    if (empty(name) && empty(value)) {
        PLOG_ERROR << format("column item at row index={} does not have a name or a value", row_index);
        return false;
    }
    return true;
}

void to_json(json &root, const TemplateItemSettings &settings) {
    root = json{{"indent_size", settings.indent_size},
                {"row_index", settings.row_index},
                {"column", settings.column},
                {"prepend_newlines", settings.prepend_newlines},
                {"append_newlines", settings.append_newlines},
                {"is_repeatable", settings.is_repeatable},
                {"is_optional", settings.is_optional},
                {"name_width", settings.name_width},
                {"name", settings.name},
                {"name_color", mmotd::tty_template::color::to_string(settings.name_color)},
                {"value", settings.value},
                {"value_color", mmotd::tty_template::color::to_string(settings.value_color)}};
}

void from_json(const json &root, TemplateItemSettings &settings, const TemplateItemSettings *default_settings) {
    if (root.contains("indent_size")) {
        root.at("indent_size").get_to(settings.indent_size);
    } else if (default_settings != nullptr) {
        settings.indent_size = default_settings->indent_size;
    }
    if (root.contains("row_index")) {
        // no default taken from default_settings
        root.at("row_index").get_to(settings.row_index);
    } else if (default_settings != nullptr) {
        // default_settings == nullptr is only true when creating default_settings
        PLOG_WARNING << "item missing row_index property (must have it for ordering purposes)";
    }
    if (root.contains("column")) {
        root.at("column").get_to(settings.column);
    } else if (default_settings != nullptr) {
        settings.column = default_settings->column;
    }
    if (root.contains("prepend_newlines")) {
        root.at("prepend_newlines").get_to(settings.prepend_newlines);
    } else if (default_settings != nullptr) {
        settings.prepend_newlines = default_settings->prepend_newlines;
    }
    if (root.contains("append_newlines")) {
        root.at("append_newlines").get_to(settings.append_newlines);
    } else if (default_settings != nullptr) {
        settings.append_newlines = default_settings->append_newlines;
    }
    if (root.contains("is_repeatable")) {
        root.at("is_repeatable").get_to(settings.is_repeatable);
    } else if (default_settings != nullptr) {
        settings.is_repeatable = default_settings->is_repeatable;
    }
    if (root.contains("is_optional")) {
        root.at("is_optional").get_to(settings.is_optional);
    } else if (default_settings != nullptr) {
        settings.is_optional = default_settings->is_optional;
    }
    if (root.contains("name_width")) {
        root.at("name_width").get_to(settings.name_width);
    } else if (default_settings != nullptr) {
        settings.name_width = default_settings->name_width;
    }
    if (root.contains("name")) {
        // no default taken from default_settings
        root.at("name").get_to(settings.name);
    } else if (default_settings != nullptr) {
        // default_settings == nullptr is only true when creating default_settings
        PLOG_WARNING << "item missing name property (should leave an empty [] name for completeness)";
    }
    if (root.contains("name_color")) {
        settings.name_color = color::from_color_string(root.at("name_color"));
    } else if (default_settings != nullptr) {
        settings.name_color = default_settings->name_color;
    }
    if (root.contains("value")) {
        // no default taken from default_settings
        root.at("value").get_to(settings.value);
    } else if (default_settings != nullptr) {
        // default_settings == nullptr is only true when creating default_settings
        PLOG_WARNING << "item missing value property (should leave an empty [] value for completeness)";
    }
    if (root.contains("value_color")) {
        settings.value_color = color::from_color_string(root.at("value_color"));
    } else if (default_settings != nullptr) {
        settings.value_color = default_settings->value_color;
    }
}

void from_json(const json &root, TemplateItemSettings &settings) {
    from_json(root, settings, nullptr);
}

string TemplateConfig::to_string() const {
    auto columns_str = format("columns = {}\n", columns_to_string());
    auto settings_str = format("default_settings = [\n{}]", default_settings_to_string());
    return columns_str + settings_str;
}

string TemplateConfig::default_settings_to_string() const {
    return default_settings.to_string();
}

string TemplateConfig::columns_to_string() const {
    using mmotd::algorithms::join;
    return format("[{}]", join(columns, ", ", [](int value) { return std::to_string(value); }));
}

void from_json(const json &root, TemplateConfig &template_config) {
    if (root.contains("columns")) {
        root.at("columns").get_to(template_config.columns);
    }
    if (root.contains("default_settings")) {
        root.at("default_settings").get_to(template_config.default_settings);
    }
}

void to_json(json &root, const TemplateConfig &template_config) {
    root = json{{"columns", template_config.columns}, {"default_settings", template_config.default_settings}};
}

string to_string(const TemplateColumnItems &items) {
    return mmotd::algorithms::join(items, "\n", [](const auto &item) {
        return format("item: [\n{}]\n", item.to_string());
    });
}

// void from_json(const json &root, TemplateColumnItems &template_column_items) {
//     if (root.contains("column_items")) {
//         root.at("column_items").get_to(template_column_items);
//     }
// }

// void to_json(nlohmann::json &root, const TemplateColumnItems &settings) {
//     root = json{{"columns", template_config.columns},
//                 {"default_settings", template_config.default_settings}};
// }

} // namespace mmotd::tty_template::data
