#include "colorized-output.h"

#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <regex>
#include <ostream>

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

int GetCodeForBeginTag(const string &tag) {
    auto i = find_if(begin(gColorMap), end(gColorMap), [&tag](const auto &color_map) {
        return tag == color_map.name;
    });
    return i != end(gColorMap) ? i->raw_code : -1;
}

string replace_begin_tag(const string &tag) {
    auto code = GetCodeForBeginTag(tag);
    return code == -1 ? string{} : format("\033[{}m", code);
}

string replace_end_tag(const string &tag) {
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

}

string color::StyleWrapper::AddStyle() const {
    auto wrapped_str = str_;
    if (style_.GetForegroundStyle() != fg_style::standard) {
        auto foreground = style_.GetForegroundStyle();
        auto i = find_if(begin(gColorMap), end(gColorMap), [foreground](const auto &color_map) {
            return foreground == static_cast<fg_style>(color_map.raw_code);
        });
        if (i != end(gColorMap)) {
            wrapped_str = format("<{0}>{1}</{0}>", i->name, wrapped_str);
        }
    }
    if (style_.GetBackgroundStyle() != bg_style::standard) {
        auto background = style_.GetBackgroundStyle();
        auto i = find_if(begin(gColorMap), end(gColorMap), [background](const auto &color_map) {
            return background == static_cast<bg_style>(color_map.raw_code);
        });
        if (i != end(gColorMap)) {
            wrapped_str = format("<{0}>{1}</{0}>", i->name, wrapped_str);
        }
    }
    if (style_.GetStyleModifier() != style_modifier::none) {
        auto modifier = style_.GetStyleModifier();
        auto i = find_if(begin(gColorMap), end(gColorMap), [modifier](const auto &color_map) {
            return modifier == static_cast<style_modifier>(color_map.raw_code);
        });
        if (i != end(gColorMap)) {
            wrapped_str = format("<{0}>{1}</{0}>", i->name, wrapped_str);
        }
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

/*
printf("\033[%d;%dm%s\033[0m\n", fg, bg, GetOutputStr(fg, bg).c_str());

echo -e "\033[33;1mTesting Testing Testing\033[0m"

printf("\033[%d;%dm%s\033[0m\n", fg, bg, GetOutputStr(fg, bg).c_str());

PS1='\e[33;1m\u@\h: \e[31m\W\e[0m\$ '
*/

string color::apply_background_foreground(string input, const char *background, const char *foreground) {
    if (background == nullptr && foreground == nullptr) {
        return input;
    } else if (background != nullptr && foreground != nullptr) {
        return format("<{0}><{1}>{2}</{1}></{0}>", background, foreground, input);
    } else if (background != nullptr) {
        return format("<{0}>{1}</{0}>", background, input);
    } else {
        return format("<{0}>{1}</{0}>", foreground, input);
    }
}

string color::apply_background(string input, const char *background) {
    return color::apply_background_foreground(input, background, nullptr);
}

string color::apply_foreground(string input, const char *foreground) {
    return color::apply_background_foreground(input, nullptr, foreground);
}


string color::to_colorized_string(string input) {
    return string{};
}
