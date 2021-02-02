// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"
#include "common/include/tty_template_substring_range.h"

#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <fmt/color.h>

namespace mmotd::tty_template::tty_string {

using ColorDefinitions = std::vector<std::string>;
std::string color_definitions_to_string(const ColorDefinitions &color_definitions);

class TemplateSubstring {
public:
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(TemplateSubstring);
    TemplateSubstring(std::string text,
                      SubstringRange prefix,
                      SubstringRange substring_text,
                      ColorDefinitions color_definitions);

    static bool IsResetColorDefinition(std::string color);

    std::string GetPrefix() const;
    std::string GetSuffix() const;
    std::string GetSubstring() const;
    SubstringRange GetSubstringRange() const;
    ColorDefinitions GetColorDefinitions() const;

    void SetPrefix(SubstringRange prefix);
    void SetSuffix(SubstringRange suffix);
    void SetSubstringText(SubstringRange text);
    void SetColorDefinitions(ColorDefinitions color_definitions);

    std::string to_string() const;
    std::string to_string(std::function<fmt::text_style(std::string)> convert_color) const;

private:
    std::string GetRawText() const { return text_; }

    static void SetValueSubstringRange(const SubstringRange &new_range,
                                       SubstringRange &existing_range,
                                       std::string name,
                                       std::string text);
    std::string text_;
    SubstringRange prefix_;
    SubstringRange substring_text_;
    ColorDefinitions color_definitions_;
    SubstringRange suffix_;

    inline static constexpr const std::string_view COLOR_RESET = "color:reset()";
};

} // namespace mmotd::tty_template::tty_string
