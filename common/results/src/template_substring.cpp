// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
// #include "common/include/app_options.h"
#include "common/include/coloring.h"
#include "common/include/config_options.h"
#include "common/include/logging.h"
#include "common/results/include/template_substring.h"

#include <iterator>
#include <string>
#include <string_view>
#include <utility>

#include <boost/algorithm/string.hpp>
#include <boost/range/iterator.hpp>
#include <boost/range/iterator_range.hpp>
#include <fmt/color.h>

using fmt::format;
using namespace std;
using mmotd::core::ConfigOptions;

namespace mmotd::results {

string color_definitions_to_string(const ColorDefinitions &color_definitions) {
    return boost::join(color_definitions, ", ");
}

TemplateSubstring::TemplateSubstring(string text,
                                     SubstringRange prefix,
                                     SubstringRange substring_text,
                                     ColorDefinitions color_definitions) :
    text_(text),
    prefix_(prefix),
    substring_text_(substring_text),
    color_definitions_(std::move(color_definitions)),
    suffix_() {
    if (!substring_text_.match(text)) {
        substring_text_.clear();
    }
    if (!prefix_.match(text)) {
        prefix_.clear();
    }
}

void TemplateSubstring::SetValueSubstringRange(const SubstringRange &new_range,
                                               SubstringRange &existing_range,
                                               string name,
                                               string text) {
    if (!new_range.match(text)) {
        return;
    } else if (existing_range.empty()) {
        LOG_VERBOSE("{} was empty, now=[{},{}], text=\"{}\"", name, new_range.position(), new_range.size(), text);
    } else {
        LOG_VERBOSE("{} was=[{},{}], now=[{},{}], text=\"{}\"",
                    name,
                    existing_range.position(),
                    existing_range.size(),
                    new_range.position(),
                    new_range.size(),
                    text);
    }
    existing_range = new_range;
}

bool TemplateSubstring::IsResetColorDefinition(string color) {
    return boost::iequals(color, COLOR_RESET);
}

string TemplateSubstring::GetPrefix() const {
    auto prefix = prefix_.substr(GetRawText());
    return prefix.has_value() ? *prefix : string{};
}

string TemplateSubstring::GetSuffix() const {
    auto suffix = suffix_.substr(GetRawText());
    return suffix.has_value() ? *suffix : string{};
}

string TemplateSubstring::GetSubstring() const {
    auto substring_text = substring_text_.substr(GetRawText());
    return substring_text.has_value() ? *substring_text : string{};
}

SubstringRange TemplateSubstring::GetSubstringRange() const {
    return substring_text_;
}

ColorDefinitions TemplateSubstring::GetColorDefinitions() const {
    return color_definitions_;
}

void TemplateSubstring::SetPrefix(SubstringRange prefix) {
    SetValueSubstringRange(prefix, prefix_, "prefix", GetRawText());
    LOG_VERBOSE("updated prefix, template substring=\"{}\"", to_string(nullptr));
}

void TemplateSubstring::SetSuffix(SubstringRange suffix) {
    SetValueSubstringRange(suffix, suffix_, "suffix", GetRawText());
    LOG_VERBOSE("updated suffix, template substring=\"{}\"", to_string());
}

void TemplateSubstring::SetSubstringText(SubstringRange substring_text) {
    SetValueSubstringRange(substring_text, substring_text_, "substring_text", GetRawText());
    LOG_VERBOSE("updated substring text, template substring=\"{}\"", to_string());
}

void TemplateSubstring::SetColorDefinitions(ColorDefinitions color_definitions) {
    color_definitions_ = color_definitions;
    LOG_VERBOSE("set color definition={} in text=\"{}\"",
                color_definitions_to_string(color_definitions_),
                GetRawText());
}

string TemplateSubstring::to_string() const {
    auto substring_output = string{};
    if (auto prefix = GetPrefix(); !std::empty(prefix)) {
        substring_output += prefix;
    }
    auto colors = GetColorDefinitions();
    for_each(std::cbegin(colors), std::cend(colors), [&substring_output](const auto &color) {
        substring_output += format(FMT_STRING("[{}]"), color);
    });
    substring_output += GetSubstring();
    if (auto suffix = GetSuffix(); !std::empty(suffix)) {
        substring_output += suffix;
    }
    return substring_output;
}

string TemplateSubstring::to_string(function<fmt::text_style(string)> convert_color) const {
    using mmotd::core::colors::RemoveNonAsciiCharsCopy;
    LOG_VERBOSE("prefix=\"{}\", color=\"{}\", text=\"{}\", suffix=\"{}\"",
                GetPrefix(),
                color_definitions_to_string(GetColorDefinitions()),
                GetSubstring(),
                GetSuffix());
    static const auto color_output = ConfigOptions::Instance().GetValueAsBooleanOr("cli.color_output", false);

    if (!color_output) {
        return RemoveNonAsciiCharsCopy(GetPrefix() + GetSubstring() + GetSuffix());
    }

    auto substring_text = GetSubstring();
    const auto &colors = GetColorDefinitions();
    for_each(begin(colors), end(colors), [&convert_color, &substring_text](const auto &color) {
        if (!empty(substring_text)) {
            substring_text = format(convert_color(color), FMT_STRING("{}"), substring_text);
        }
    });
    return GetPrefix() + substring_text + GetSuffix();
}

} // namespace mmotd::results
