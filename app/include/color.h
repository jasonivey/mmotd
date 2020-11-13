// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <iosfwd>
#include <ostream>
#include <string>
#include <string_view>

namespace color {

void PrintError(std::ostream &output_stream, const char *message, bool tee_to_log = true);
void PrintError(std::ostream &output_stream, std::string message, bool tee_to_log = true);
void PrintError(std::ostream &output_stream, std::string_view message, bool tee_to_log = true);
void PrintError(const char *message, bool tee_to_log = true);
void PrintError(std::string message, bool tee_to_log = true);
void PrintError(std::string_view message, bool tee_to_log = true);

void PrintInfo(std::ostream &output_stream, const char *message, bool tee_to_log = true);
void PrintInfo(std::ostream &output_stream, std::string message, bool tee_to_log = true);
void PrintInfo(std::ostream &output_stream, std::string_view message, bool tee_to_log = true);
void PrintInfo(const char *message, bool tee_to_log = true);
void PrintInfo(std::string message, bool tee_to_log = true);
void PrintInfo(std::string_view message, bool tee_to_log = true);

// For better compability with most terminals avoid style_modifier's except
//  reset, bold and reversed.  The only ones I've confirmed working are
//  bold, dim, underline, reversed and crossed.
//
// NOTE: the StyleWrapper automatically adds the reset for any styles previously
//       added by style_modifier, fg_style and bg_style.
//
enum class style_modifier : int {
    reset = 0,     // resets the styles by removing all changes
    bold = 1,      // makes the characters thicker
    dim = 2,       // makes the text faint
    italic = 3,    // does nothing
    underline = 4, // underlines the text
    blink = 5,     // does nothing
    rblink = 6,    // does nothing
    reversed = 7,  // reverses the background and foreground colors
    conceal = 8,   // does nothing
    crossed = 9,   // crosses through the text
    none = 10
};

enum class fg_style : int {
    black = 30,
    red = 31,
    green = 32,
    yellow = 33,
    blue = 34,
    magenta = 35,
    cyan = 36,
    white = 37,
    standard = 39,
    bright_black = 90,
    bright_red = 91,
    bright_green = 92,
    bright_yellow = 93,
    bright_blue = 94,
    bright_magenta = 95,
    bright_cyan = 96,
    bright_white = 97
};

enum class bg_style : int {
    black = 40,
    red = 41,
    green = 42,
    yellow = 43,
    blue = 44,
    magenta = 45,
    cyan = 46,
    white = 47,
    standard = 49,
    bright_black = 100,
    bright_red = 101,
    bright_green = 102,
    bright_yellow = 103,
    bright_blue = 104,
    bright_magenta = 105,
    bright_cyan = 106,
    bright_white = 107
};

// Style object: a container for the foreground color, background color and misc modifiers
class Style {
public:
    constexpr Style() = default;
    constexpr Style(fg_style foreground, style_modifier modifier, bg_style background) :
        foreground_(foreground), modifier_(modifier), background_(background) {}

    fg_style GetForegroundStyle() const { return foreground_; }
    style_modifier GetStyleModifier() const { return modifier_; }
    bg_style GetBackgroundStyle() const { return background_; }

private:
    fg_style foreground_ = fg_style::standard;
    style_modifier modifier_ = style_modifier::none;
    bg_style background_ = bg_style::standard;
};

// StyleWrapper object: an object which is able to encode/decode a string with color markers or console escape sequences
#if 0
template<typename T>
class StyleWrapper {
    template<typename T>
    friend std::ostream &operator<<(std::ostream &, const StyleWrapper<T> &);

public:
    StyleWrapper() = default;
    template<typename T>
    StyleWrapper(T str, Style style) : str_(str), style_(style) {}

    // returns str{"hello world"} wrapped in style -> str{"<FgRed><BgWhite>hello world</BgWhite></FgRed>"}
    std::string Encoded() const { return ReplaceStyle(); }
    std::string Decoded() const { return AddStyle(); }

    // returns the string ready to be printed to the terminal -> str{"\033[31m\033[47mhello world\033[0m\033[0m"}
    std::string to_string() const { return Encoded(); }

    size_t RawSize() const { return str_.size(); }
    size_t WrappedSize() const { return Decoded().size(); }
    size_t DeltaSize() const { return Decoded().size() - str_.size(); }

    void SetStyle(Style style) { style_ = style; }

    template<typename T>
    void SetStr(T str) { str_ = str; }

private:
    std::string AddStyle() const;
    std::string ReplaceStyle() const;

    T str_;
    Style style_;
};
#else
class StyleWrapper {
    friend std::ostream &operator<<(std::ostream &, const StyleWrapper &);

public:
    StyleWrapper() = default;
    StyleWrapper(std::string str, Style style) : str_(str), style_(style) {}

    // returns str{"hello world"} wrapped in style -> str{"<FgRed><BgWhite>hello world</BgWhite></FgRed>"}
    std::string Encoded() const { return ReplaceStyle(); }
    std::string Decoded() const { return AddStyle(); }

    // returns the string ready to be printed to the terminal -> str{"\033[31m\033[47mhello world\033[0m\033[0m"}
    std::string to_string() const { return Encoded(); }

    size_t RawSize() const { return str_.size(); }
    size_t WrappedSize() const { return Decoded().size(); }
    size_t DeltaSize() const { return Decoded().size() - str_.size(); }

    void SetStyle(Style style) { style_ = style; }

    void SetStr(std::string str) { str_ = str; }

private:
    std::string AddStyle() const;
    std::string ReplaceStyle() const;

    std::string str_;
    Style style_;
};
#endif

// make_style: creates the Style object which wraps the foreground color, background color and misc modifiers
inline constexpr Style make_style(fg_style foreground, style_modifier modifier, bg_style background) {
    return Style{foreground, modifier, background};
}

inline constexpr Style make_style(fg_style foreground, style_modifier modifier) {
    return Style{foreground, modifier, bg_style::standard};
}

inline constexpr Style make_style(fg_style foreground, bg_style background) {
    return Style{foreground, style_modifier::none, background};
}

inline constexpr Style make_style(fg_style foreground) {
    return Style{foreground, style_modifier::none, bg_style::standard};
}

inline constexpr Style make_style(style_modifier modifier, bg_style background) {
    return Style{fg_style::standard, modifier, background};
}

inline constexpr Style make_style(style_modifier modifier) {
    return Style{fg_style::standard, modifier, bg_style::standard};
}

inline constexpr Style make_style(bg_style background) {
    return Style{fg_style::standard, style_modifier::none, background};
}

// make_style_wrapper: creates an object which is able to encode/decode a string with color markers or console escape
// sequences
inline StyleWrapper
make_style_wrapper(std::string str, fg_style foreground, style_modifier modifier, bg_style background) {
    return StyleWrapper(str, make_style(foreground, modifier, background));
}

inline StyleWrapper make_style_wrapper(std::string str, fg_style foreground, style_modifier modifier) {
    return StyleWrapper(str, make_style(foreground, modifier, bg_style::standard));
}

inline StyleWrapper make_style_wrapper(std::string str, fg_style foreground, bg_style background) {
    return StyleWrapper(str, make_style(foreground, style_modifier::none, background));
}

inline StyleWrapper make_style_wrapper(std::string str, fg_style foreground) {
    return StyleWrapper(str, make_style(foreground, style_modifier::none, bg_style::standard));
}

inline StyleWrapper make_style_wrapper(std::string str, style_modifier modifier, bg_style background) {
    return StyleWrapper(str, make_style(fg_style::standard, modifier, background));
}

inline StyleWrapper make_style_wrapper(std::string str, style_modifier modifier) {
    return StyleWrapper(str, make_style(fg_style::standard, modifier, bg_style::standard));
}

inline StyleWrapper make_style_wrapper(std::string str, bg_style background) {
    return StyleWrapper(str, make_style(fg_style::standard, style_modifier::none, background));
}

inline std::string
make_stylized_string(std::string str, fg_style foreground, style_modifier modifier, bg_style background) {
    return StyleWrapper(str, make_style(foreground, modifier, background)).to_string();
}

inline std::string make_stylized_string(std::string str, fg_style foreground, style_modifier modifier) {
    return StyleWrapper(str, make_style(foreground, modifier, bg_style::standard)).to_string();
}

inline std::string make_stylized_string(std::string str, fg_style foreground, bg_style background) {
    return StyleWrapper(str, make_style(foreground, style_modifier::none, background)).to_string();
}

inline std::string make_stylized_string(std::string str, fg_style foreground) {
    return StyleWrapper(str, make_style(foreground, style_modifier::none, bg_style::standard)).to_string();
}

inline std::string make_stylized_string(std::string str, style_modifier modifier, bg_style background) {
    return StyleWrapper(str, make_style(fg_style::standard, modifier, background)).to_string();
}

inline std::string make_stylized_string(std::string str, style_modifier modifier) {
    return StyleWrapper(str, make_style(fg_style::standard, modifier, bg_style::standard)).to_string();
}

inline std::string make_stylized_string(std::string str, bg_style background) {
    return StyleWrapper(str, make_style(fg_style::standard, style_modifier::none, background)).to_string();
}

} // namespace color
