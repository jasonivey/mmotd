// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "app/include/color.h"

#include <boost/log/trivial.hpp>
#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <regex>
#include <optional>
#include <ostream>
#include <cstdio>
#include <iostream>

constexpr const char *StyleReset = "StyleReset";
constexpr const char *StyleBold = "StyleBold";
constexpr const char *StyleDim = "StyleDim";
constexpr const char *StyleItalic = "StyleItalic";
constexpr const char *StyleUnderline = "StyleUnderline";
constexpr const char *StyleBlink = "StyleBlink";
constexpr const char *StyleRblink = "StyleRblink";
constexpr const char *StyleReversed = "StyleReversed";
constexpr const char *StyleConceal = "StyleConceal";
constexpr const char *StyleCrossed = "StyleCrossed";


constexpr const char *FgBlack = "FgBlack";
constexpr const char *FgRed = "FgRed";
constexpr const char *FgGreen = "FgGreen";
constexpr const char *FgYellow = "FgYellow";
constexpr const char *FgBlue = "FgBlue";
constexpr const char *FgMagenta = "FgMagenta";
constexpr const char *FgCyan = "FgCyan";
constexpr const char *FgWhite = "FgWhite";
constexpr const char *FgBrightBlack = "FgBrightBlack";
constexpr const char *FgBrightRed = "FgBrightRed";
constexpr const char *FgBrightGreen = "FgBrightGreen";
constexpr const char *FgBrightYellow = "FgBrightYellow";
constexpr const char *FgBrightBlue = "FgBrightBlue";
constexpr const char *FgBrightMagenta = "FgBrightMagenta";
constexpr const char *FgBrightCyan = "FgBrightCyan";
constexpr const char *FgBrightWhite = "FgBrightWhite";

constexpr const char *BgBlack = "BgBlack";
constexpr const char *BgRed = "BgRed";
constexpr const char *BgGreen = "BgGreen";
constexpr const char *BgYellow = "BgYellow";
constexpr const char *BgBlue = "BgBlue";
constexpr const char *BgMagenta = "BgMagenta";
constexpr const char *BgCyan = "BgCyan";
constexpr const char *BgWhite = "BgWhite";
constexpr const char *BgBrightBlack = "BgBrightBlack";
constexpr const char *BgBrightRed = "BgBrightRed";
constexpr const char *BgBrightGreen = "BgBrightGreen";
constexpr const char *BgBrightYellow = "BgBrightYellow";
constexpr const char *BgBrightBlue = "BgBrightBlue";
constexpr const char *BgBrightMagenta = "BgBrightMagenta";
constexpr const char *BgBrightCyan = "BgBrightCyan";
constexpr const char *BgBrightWhite = "BgBrightWhite";

using namespace std;
using namespace fmt;

static constexpr const char *BEGIN_XML_TAG = "<([^/>]+)>";
static constexpr const char *END_XML_TAG = "</([^>]+)>";

namespace {

struct ColorMap {
    int raw_code = 0;
    const char *name;
};

static constexpr array<ColorMap, 42> gColorMap = {
    ColorMap{static_cast<int>(color::style_modifier::reset), StyleReset},
    ColorMap{static_cast<int>(color::style_modifier::bold), StyleBold},
    ColorMap{static_cast<int>(color::style_modifier::dim), StyleDim},
    ColorMap{static_cast<int>(color::style_modifier::italic), StyleItalic},
    ColorMap{static_cast<int>(color::style_modifier::underline), StyleUnderline},
    ColorMap{static_cast<int>(color::style_modifier::blink), StyleBlink},
    ColorMap{static_cast<int>(color::style_modifier::rblink), StyleRblink},
    ColorMap{static_cast<int>(color::style_modifier::reversed), StyleReversed},
    ColorMap{static_cast<int>(color::style_modifier::conceal), StyleConceal},
    ColorMap{static_cast<int>(color::style_modifier::crossed), StyleCrossed},

    ColorMap{static_cast<int>(color::fg_style::black), FgBlack},
    ColorMap{static_cast<int>(color::fg_style::red), FgRed},
    ColorMap{static_cast<int>(color::fg_style::green), FgGreen},
    ColorMap{static_cast<int>(color::fg_style::yellow), FgYellow},
    ColorMap{static_cast<int>(color::fg_style::blue), FgBlue},
    ColorMap{static_cast<int>(color::fg_style::magenta), FgMagenta},
    ColorMap{static_cast<int>(color::fg_style::cyan), FgCyan},
    ColorMap{static_cast<int>(color::fg_style::white), FgWhite},
    ColorMap{static_cast<int>(color::fg_style::bright_black), FgBrightBlack},
    ColorMap{static_cast<int>(color::fg_style::bright_red), FgBrightRed},
    ColorMap{static_cast<int>(color::fg_style::bright_green), FgBrightGreen},
    ColorMap{static_cast<int>(color::fg_style::bright_yellow), FgBrightYellow},
    ColorMap{static_cast<int>(color::fg_style::bright_blue), FgBrightBlue},
    ColorMap{static_cast<int>(color::fg_style::bright_magenta), FgBrightMagenta},
    ColorMap{static_cast<int>(color::fg_style::bright_cyan), FgBrightCyan},
    ColorMap{static_cast<int>(color::fg_style::bright_white), FgBrightWhite},

    ColorMap{static_cast<int>(color::bg_style::black), BgBlack},
    ColorMap{static_cast<int>(color::bg_style::red), BgRed},
    ColorMap{static_cast<int>(color::bg_style::green), BgGreen},
    ColorMap{static_cast<int>(color::bg_style::yellow), BgYellow},
    ColorMap{static_cast<int>(color::bg_style::blue), BgBlue},
    ColorMap{static_cast<int>(color::bg_style::magenta), BgMagenta},
    ColorMap{static_cast<int>(color::bg_style::cyan), BgCyan},
    ColorMap{static_cast<int>(color::bg_style::white), BgWhite},
    ColorMap{static_cast<int>(color::bg_style::bright_black), BgBrightBlack},
    ColorMap{static_cast<int>(color::bg_style::bright_red), BgBrightRed},
    ColorMap{static_cast<int>(color::bg_style::bright_green), BgBrightGreen},
    ColorMap{static_cast<int>(color::bg_style::bright_yellow), BgBrightYellow},
    ColorMap{static_cast<int>(color::bg_style::bright_blue), BgBrightBlue},
    ColorMap{static_cast<int>(color::bg_style::bright_magenta), BgBrightMagenta},
    ColorMap{static_cast<int>(color::bg_style::bright_cyan), BgBrightCyan},
    ColorMap{static_cast<int>(color::bg_style::bright_white), BgBrightWhite}
};

template<typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
const ColorMap *FindColorMap(T code) {
    auto i = find_if(begin(gColorMap), end(gColorMap), [code](const auto &color_map) {
        return static_cast<int>(code) == color_map.raw_code;
    });
    return i != end(gColorMap) ? &(*i) : nullptr;
}

const ColorMap *FindColorMap(const char *name) {
    auto i = find_if(begin(gColorMap), end(gColorMap), [name](const auto &color_map) {
        return strcmp(color_map.name, name) == 0;
    });
    return i != end(gColorMap) ? &(*i) : nullptr;
}

template<typename T>
optional<const char *> GetTagNameForCode(T code) {
    const auto *color_map = FindColorMap(code);
    return color_map == nullptr ? nullopt : make_optional(color_map->name);
}

optional<int> GetCodeForTagName(const string &tag) {
    const auto *color_map = FindColorMap(tag.c_str());
    return color_map == nullptr ? nullopt : make_optional(color_map->raw_code);
}

string replace_begin_tag(const string &tag) {
    auto code = GetCodeForTagName(tag);
    return code.has_value() ? format("\033[{}m", *code) : string();
}

string replace_end_tag(const string &) {
    return "\033[0m";
}

template<typename T>
string replace_tags(string input, const char *tag, T replace_func) {
    auto begin_tag_regex = regex(tag);
    auto begin_iterator = sregex_iterator(begin(input), end(input), begin_tag_regex);
    auto end_iterator = sregex_iterator();

    auto output = string{};
    for (auto i = begin_iterator; i != end_iterator; ++i) {
        const auto &match = *i;
        output += match.prefix().str();
        output += replace_func(match.str(1));
        if (std::distance(i, end_iterator) == 1) {
            output += match.suffix().str();
        }
    }
    return output;
}

template<typename T>
string replace_end_tags(string input, T end_replace_func) {
    return replace_tags(input, END_XML_TAG, end_replace_func);
}

template<typename T>
string replace_begin_tags(string input, T begin_replace_func) {
    return replace_tags(input, BEGIN_XML_TAG, begin_replace_func);
}

template<typename T, typename U>
string replace_tags(string input, T begin_replace_func, U end_replace_func) {
    return replace_end_tags(replace_begin_tags(input, begin_replace_func), end_replace_func);
}

void PrintErrorInternal(ostream &out, std::string msg, bool tee_to_log) {
    auto str_msg = string{msg};
    auto colorized_msg = color::make_stylized_string(str_msg, color::fg_style::bright_white);
    auto colorized_prefix = color::make_stylized_string("ERROR: ", color::fg_style::red, color::style_modifier::bold);
    out << colorized_prefix << colorized_msg;
    if (msg.back() != '\n') {
        out << "\n";
    }
    if (tee_to_log) {
        BOOST_LOG_TRIVIAL(error) << msg;
    }
}

void PrintErrorInternal(ostream &out, const char *msg, bool tee_to_log) {
    auto colorized_msg = color::make_stylized_string(msg, color::fg_style::bright_white);
    auto colorized_prefix = color::make_stylized_string("ERROR: ", color::fg_style::red, color::style_modifier::bold);
    out << colorized_prefix << colorized_msg;
    if (msg[strlen(msg) - 1] != '\n') {
        out << "\n";
    }
    if (tee_to_log) {
        BOOST_LOG_TRIVIAL(error) << msg;
    }
}

void PrintErrorInternal(ostream &out, std::string_view msg, bool tee_to_log) {
    auto str_msg = string{msg};
    auto colorized_msg = color::make_stylized_string(str_msg, color::fg_style::bright_white);
    auto colorized_prefix = color::make_stylized_string("ERROR: ", color::fg_style::red, color::style_modifier::bold);
    out << colorized_prefix << colorized_msg;
    if (msg.back() != '\n') {
        out << "\n";
    }
    if (tee_to_log) {
        BOOST_LOG_TRIVIAL(error) << msg;
    }
}

void PrintInfoInternal(ostream &out, string msg, bool tee_to_log) {
    auto colorized_msg = color::make_stylized_string(msg, color::fg_style::bright_white);
    auto colorized_prefix = color::make_stylized_string("INFO: ", color::fg_style::green, color::style_modifier::bold);
    out << colorized_prefix << colorized_msg;
    if (msg.back() != '\n') {
        out << "\n";
    }
    if (tee_to_log) {
        BOOST_LOG_TRIVIAL(info) << msg;
    }
}

void PrintInfoInternal(ostream &out, const char *msg, bool tee_to_log) {
    auto colorized_msg = color::make_stylized_string(msg, color::fg_style::bright_white);
    auto colorized_prefix = color::make_stylized_string("INFO: ", color::fg_style::green, color::style_modifier::bold);
    out << colorized_prefix << colorized_msg;
    if (msg[strlen(msg) - 1] != '\n') {
        out << "\n";
    }
    if (tee_to_log) {
        BOOST_LOG_TRIVIAL(info) << msg;
    }
}

void PrintInfoInternal(ostream &out, std::string_view msg, bool tee_to_log) {
    auto str_msg = string{msg};
    auto colorized_msg = color::make_stylized_string(str_msg, color::fg_style::bright_white);
    auto colorized_prefix = color::make_stylized_string("INFO: ", color::fg_style::green, color::style_modifier::bold);
    out << colorized_prefix << colorized_msg;
    if (msg.back() != '\n') {
        out << "\n";
    }
    if (tee_to_log) {
        BOOST_LOG_TRIVIAL(info) << msg;
    }
}

}

string color::StyleWrapper::AddStyle() const {
    auto wrapped_str = str_;

    auto foreground = style_.GetForegroundStyle();
    auto foreground_name = GetTagNameForCode(foreground);
    if (foreground_name && foreground != fg_style::standard) {
        wrapped_str = format("<{0}>{1}</{0}>", *foreground_name, wrapped_str);
    }

    auto style_modifier = style_.GetStyleModifier();
    auto style_modifier_name = GetTagNameForCode(style_modifier);
    if (style_modifier_name && style_modifier != style_modifier::none) {
        wrapped_str = format("<{0}>{1}</{0}>", *style_modifier_name, wrapped_str);
    }

    auto background = style_.GetBackgroundStyle();
    auto background_name = GetTagNameForCode(background);
    if (background_name && background != bg_style::standard) {
        wrapped_str = format("<{0}>{1}</{0}>", *background_name, wrapped_str);
    }
    return wrapped_str;
}

string color::StyleWrapper::ReplaceStyle() const {
    auto wrapped_str = AddStyle();
    return replace_tags(wrapped_str, replace_begin_tag, replace_end_tag);
}

ostream &operator<<(ostream &out, const color::StyleWrapper &style_wrapper) {
    out << style_wrapper.to_string();
    return out;
}

void color::PrintError(ostream &output_stream, const char *msg, bool tee_to_log) {
    PrintErrorInternal(output_stream, msg, tee_to_log);
}

void color::PrintError(ostream &output_stream, string msg, bool tee_to_log) {
    PrintErrorInternal(output_stream, msg, tee_to_log);
}

void color::PrintError(ostream &output_stream, std::string_view msg, bool tee_to_log) {
    PrintErrorInternal(output_stream, msg, tee_to_log);
}

void color::PrintError(const char *msg, bool tee_to_log) {
    PrintErrorInternal(cerr, msg, tee_to_log);
}

void color::PrintError(string msg, bool tee_to_log) {
    PrintErrorInternal(cerr, msg, tee_to_log);
}

void color::PrintError(std::string_view msg, bool tee_to_log) {
    PrintErrorInternal(cerr, msg, tee_to_log);
}

void color::PrintInfo(ostream &output_stream, const char *msg, bool tee_to_log) {
    PrintInfoInternal(output_stream, msg, tee_to_log);
}

void color::PrintInfo(ostream &output_stream, string msg, bool tee_to_log) {
    PrintInfoInternal(output_stream, msg, tee_to_log);
}

void color::PrintInfo(ostream &output_stream, std::string_view msg, bool tee_to_log) {
    PrintInfoInternal(output_stream, msg, tee_to_log);
}

void color::PrintInfo(const char *msg, bool tee_to_log) {
    PrintInfoInternal(cout, msg, tee_to_log);
}

void color::PrintInfo(string msg, bool tee_to_log) {
    PrintInfoInternal(cout, msg, tee_to_log);
}

void color::PrintInfo(std::string_view msg, bool tee_to_log) {
    PrintInfoInternal(cout, msg, tee_to_log);
}

/*
printf("\033[%d;%dm%s\033[0m\n", fg, bg, GetOutputStr(fg, bg).c_str());

echo -e "\033[33;1mTesting Testing Testing\033[0m"

printf("\033[%d;%dm%s\033[0m\n", fg, bg, GetOutputStr(fg, bg).c_str());

PS1='\e[33;1m\u@\h: \e[31m\W\e[0m\$ '
*/
