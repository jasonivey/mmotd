// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/algorithm.h"
#include "common/include/information.h"
#include "common/include/information_decls.h"
#include "common/include/informations.h"
#include "common/include/tty_template_data.h"
#include "common/include/tty_template_string.h"
#include "common/include/tty_template_substring.h"
#include "common/include/tty_template_substring_range.h"
#include "common/include/tty_template_substrings.h"

#include <algorithm>
#include <iterator>
#include <optional>
#include <regex>
#include <tuple>

#include <boost/algorithm/string.hpp>
#include <fmt/color.h>
#include <plog/Log.h>

using namespace std;
using fmt::format;
using mmotd::algorithms::collect_if;
using mmotd::information::Information;
using mmotd::information::InformationId;
using mmotd::information::Informations;
using mmotd::tty_template::data::TemplateColumnItem;

namespace mmotd::tty_template::tty_string {

TemplateString::TemplateString(string text) : text_(text) {
}

optional<Information> TemplateString::FindInformation(const string &information_id,
                                                      const Informations &informations,
                                                      size_t information_index) {
    auto information_ptrs = vector<const Information *>{};
    collect_if(
        begin(informations),
        end(informations),
        [information_id](const auto &information) {
            const auto &information_id_str = information.to_repr();
            return boost::iequals(information_id_str, information_id) ||
                   boost::iequals(information_id_str, string{"InformationId::"} + information_id);
        },
        [&information_ptrs](const auto &information) { information_ptrs.push_back(&information); });
    if (information_index >= std::size(information_ptrs)) {
        PLOG_ERROR << format("attempting to find the index={} of id={} when there is only {} of that id",
                             information_index,
                             information_id,
                             std::size(information_ptrs));
        return nullopt;
    } else if (information_ptrs[information_index] == nullptr) {
        return nullopt;
    } else {
        return make_optional(*(information_ptrs[information_index]));
    }
}

optional<string> TemplateString::GetInformationValue(const string &information_id,
                                                     const Informations &informations,
                                                     size_t information_index) {
    auto information_holder = TemplateString::FindInformation(information_id, informations, information_index);
    if (!information_holder) {
        PLOG_ERROR << format("unable to find information id {}", information_id);
        return nullopt;
    }
    const auto &information = *information_holder;
    auto information_value = information.GetValue();
    PLOG_VERBOSE << format("found {} with value {}", information_id, information_value);
    return make_optional(information_value);
}

string
TemplateString::ReplaceInformationIds(const string &text, const Informations &informations, size_t information_index) {
    if (std::empty(informations)) {
        return text;
    }

    PLOG_VERBOSE << format("template str: {}", text);
    const auto pattern = regex(R"(%(ID_[_A-Z]+)%)");
    auto replacement_token = sregex_iterator(begin(text), end(text), pattern);

    auto formatted_str = string{};
    auto index = smatch::difference_type{0};
    for (auto i = replacement_token; i != sregex_iterator{}; ++i) {
        auto match = *i;
        PLOG_VERBOSE << format("index: {}, pos: {}, len: {}, iterator: {}",
                               index,
                               match.position(),
                               match.length(),
                               match.str());
        if (match.position() > index) {
            PLOG_VERBOSE << format("adding prefix: \"{}\"", text.substr(index, match.position() - index));
            formatted_str += text.substr(index, match.position() - index);
        }
        index = match.position() + match.length();
        if (auto value = TemplateString::GetInformationValue(match.str(1), informations, information_index); value) {
            formatted_str += *value;
        }
    }
    if (index < static_cast<smatch::difference_type>(std::size(text))) {
        PLOG_VERBOSE << format("adding suffix: \"{}\"", text.substr(index));
        formatted_str += text.substr(index);
    }
    PLOG_VERBOSE << format("formatted: {}", formatted_str);
    return formatted_str;
}

InformationId TemplateString::FindFirstInformationId(const string &text, const Informations &informations) {
    const auto information_id_regex = regex(R"(%(ID_[_A-Z]+)%)");
    auto matches = smatch{};
    if (regex_search(cbegin(text), cend(text), matches, information_id_regex)) {
        if (auto info = TemplateString::FindInformation(matches.str(1), informations, 0); info) {
            return info.value().GetId();
        }
    }
    return InformationId::ID_INVALID_INVALID_INFORMATION;
}

vector<string> TemplateString::SplitColorCodeDefinitions(const string &color_str, const char delimeter) {
    if (std::empty(color_str)) {
        PLOG_VERBOSE << format("input is empty delim='{}', input=\"{}\"", delimeter, color_str);
        return vector<string>{};
    } else if (color_str.front() != delimeter || color_str.back() != delimeter) {
        PLOG_VERBOSE << format("input is not wrapped by delim='{}', input=\"{}\"", delimeter, color_str);
        return vector<string>{color_str};
    }
    auto color_definitions = vector<string>{};
    auto index = size_t{1};
    while (index != string::npos) {
        auto previous = index;
        index = color_str.find(string(size_t{2}, delimeter), index);
        if (index != string::npos) {
            auto str_begin = cbegin(color_str) + previous;
            auto str_end = cbegin(color_str) + index;
            color_definitions.push_back(string{str_begin, str_end});
            index += std::size(string(size_t{2}, delimeter));
            PLOG_VERBOSE << format("found {}, index={}, in=\"{}\", created color def=\"{}\"",
                                   string(size_t{2}, delimeter),
                                   index,
                                   color_str,
                                   color_definitions.back());
        } else {
            auto str_begin = cbegin(color_str) + previous;
            auto str_end = cend(color_str) - 1;
            color_definitions.push_back(string{str_begin, str_end});
            PLOG_VERBOSE << format("unable to find {}, index={}, in=\"{}\", created color def=\"{}\"",
                                   string(size_t{2}, delimeter),
                                   previous,
                                   color_str,
                                   color_definitions.back());
        }
    }
    return color_definitions;
}

vector<string> TemplateString::SplitMultipleColorCodeDefinitions(const vector<string> &color_defs,
                                                                 const char delimeter) {
    auto new_colors_defs = vector<string>{};
    for (const auto &color_def : color_defs) {
        if (std::empty(color_def)) {
            continue;
        }
        // color:reset():hex(FFFFFF)
        auto split_colors_defs = vector<string>{};
        boost::split(split_colors_defs, color_def, boost::is_any_of(string{delimeter}));
        copy(begin(split_colors_defs), end(split_colors_defs), back_inserter(new_colors_defs));
    }
    if (std::empty(new_colors_defs)) {
        return new_colors_defs;
    }
    auto final_colors_defs = vector<string>{};
    mmotd::algorithms::transform_if(
        begin(new_colors_defs),
        end(new_colors_defs),
        back_inserter(final_colors_defs),
        [](const auto &color_def) { return !boost::iequals(color_def, "color"); },
        [](const auto &color_def) { return "color:" + color_def; });
    return final_colors_defs;
}

vector<string> TemplateString::ParseColorCodeString(const string &color_str) {
    auto combined_color_definitions = TemplateString::SplitColorCodeDefinitions(color_str);
    return TemplateString::SplitMultipleColorCodeDefinitions(combined_color_definitions);
}

void TemplateString::CreateTemplateSubstring(const string &text,
                                             TemplateSubstrings &template_substrings,
                                             const SubstringRange &substring_text,
                                             const vector<string> &color_definitions,
                                             const SubstringRange &prefix) {
    if (substring_text.empty() || std::empty(color_definitions)) {
        return;
    }

    template_substrings.emplace_back(make_unique<TemplateSubstring>(text, prefix, substring_text, color_definitions));
    PLOG_VERBOSE << format("created template substring=\"{}\" from \"{}\"",
                           template_substrings.back().to_string(),
                           text);
}

void TemplateString::UpdateLastTemplateSubstring(const string &text,
                                                 TemplateSubstrings &template_substrings,
                                                 size_t next_template_substring_position) {
    if (std::empty(template_substrings)) {
        PLOG_VERBOSE << format("there was no previous substring to updated end marker in \"{}\"", text);
        return;
    }

    PLOG_VERBOSE << format("updating previous substring end marker to {} in \"{}\"",
                           next_template_substring_position,
                           text);
    auto previous_range = template_substrings.back().GetSubstringRange();
    auto new_range = SubstringRange{previous_range.position(), next_template_substring_position};
    if (previous_range == new_range) {
        PLOG_VERBOSE << format("both new and existing substrings are equal [{}:{}], [{}:{}] in \"{}\"",
                               previous_range.position(),
                               previous_range.size(),
                               new_range.position(),
                               new_range.size(),
                               text);
        return;
    }

    PLOG_VERBOSE << format("setting substring text from [{}:{}] to [{}:{}] in \"{}\"",
                           previous_range.position(),
                           previous_range.size(),
                           new_range.position(),
                           new_range.size(),
                           text);
    template_substrings.back().SetSubstringText(new_range);
    PLOG_VERBOSE << format("new substring text \"{}\" in \"{}\"", template_substrings.back().GetSubstring(), text);
}

bool TemplateString::IsColorCodeMatchValid(const string &text, const smatch &matches) {
    if (static_cast<size_t>(matches.position()) > std::size(text) ||
        static_cast<size_t>(matches.position() + matches.length()) > std::size(text)) {
        PLOG_ERROR << format("color definition is beyond EOS, match start={}, match len={}, str size={}, str={}",
                             matches.position(),
                             matches.length(),
                             std::size(text),
                             text);
        return false;
    }
    PLOG_VERBOSE << format("color definition is valid start={}, len={}, str size={}, match={}, str={}",
                           matches.position(),
                           matches.length(),
                           std::size(text),
                           text.substr(matches.position(), matches.length()),
                           text);
    return true;
}

void TemplateString::ParseColorCodeSubstring(const string &text,
                                             size_t &index,
                                             TemplateSubstrings &template_substrings) {
    static const auto regex_color_pattern = regex{R"((?:%color[^%]+%)+)"};
    auto matches = smatch{};

    if (!regex_search(cbegin(text) + index, cend(text), matches, regex_color_pattern)) {
        auto input_substr = string_view{text.c_str() + index};
        PLOG_VERBOSE << format("color code not found in \"{}\"", input_substr);
        TemplateString::UpdateLastTemplateSubstring(text, template_substrings, std::size(text));
        index = std::size(text);
        return;
    }

    if (!TemplateString::IsColorCodeMatchValid(text, matches)) {
        index += matches.position() + matches.length();
        return;
    }

    auto match_substr =
        string{cbegin(text) + index + matches.position(), cbegin(text) + index + matches.position() + matches.length()};
    auto input_substr = string_view{text.c_str() + index};
    PLOG_VERBOSE << format("color code found match={}, text=\"{}\"", match_substr, input_substr);

    auto prefix = SubstringRange{};
    if (!std::empty(template_substrings)) {
        TemplateString::UpdateLastTemplateSubstring(text, template_substrings, matches.position());
    } else if (index < static_cast<size_t>(matches.position())) {
        prefix = SubstringRange{index, matches.position() - index};
    }

    auto color_definitions = TemplateString::ParseColorCodeString(match_substr);
    auto substring_offset = index + matches.position() + matches.length();
    auto substring_count = std::size(text) - substring_offset;
    auto substring_text = SubstringRange(substring_offset, substring_count);
    TemplateString::CreateTemplateSubstring(text, template_substrings, substring_text, color_definitions, prefix);

    index += matches.position() + matches.length();
}

TemplateSubstrings TemplateString::GenerateTemplateSubstrings(const string &text) {
    auto template_substrings = TemplateSubstrings{};
    auto loop_count = 0;
    auto index = size_t{0};
    while (index < std::size(text) && loop_count++ < 10) {
        TemplateString::ParseColorCodeSubstring(text, index, template_substrings);

        if (index == std::size(text) && std::empty(template_substrings)) {
            template_substrings.emplace_back(make_unique<TemplateSubstring>(text,
                                                                            SubstringRange{},
                                                                            SubstringRange{0ull, std::size(text)},
                                                                            ColorDefinitions{}));
        }
    }
    PLOG_ERROR_IF(loop_count == 10) << format("parse color code exited after 10 iterations, text=\"{}\"", text);
    return template_substrings;
}

fmt::text_style TemplateString::GetColorValue(string color_specification) {
    static const string color_prefix = string{"color:"};
    if (boost::istarts_with(color_specification, color_prefix)) {
        color_specification = color_specification.substr(std::size(color_prefix));
    }
    return mmotd::tty_template::color::from_color_string(color_specification);
}

string TemplateString::ReplaceEmbeddedColorCodes(TemplateType template_type,
                                                 const string &text,
                                                 const TemplateColumnItem &item) {
    auto template_substrings = TemplateString::GenerateTemplateSubstrings(text);
    auto formatted_str = template_substrings.to_string(&TemplateString::GetColorValue);
    auto color_style = template_type == TemplateType::Name ? item.name_color : item.value_color;
    return format(color_style, formatted_str);
}

string TemplateString::TransformTemplate(TemplateType template_type,
                                         const string &text,
                                         const Informations &informations,
                                         const TemplateColumnItem &item,
                                         size_t information_index) {
    auto updated_text1 = TemplateString::ReplaceInformationIds(text, informations, information_index);
    PLOG_VERBOSE << format("transforming \"{}\" to \"{}\"", text, updated_text1);
    auto updated_text2 = TemplateString::ReplaceEmbeddedColorCodes(template_type, updated_text1, item);
    PLOG_VERBOSE << format("transformed \"{}\" to \"{}\"", updated_text1, updated_text2);
    return updated_text2;
}

string
TemplateString::TransformTemplateName(const Informations &informations, const TemplateColumnItem &item, size_t index) {
    return TemplateString::TransformTemplate(TemplateType::Name, text_, informations, item, index);
}

string
TemplateString::TransformTemplateValue(const Informations &informations, const TemplateColumnItem &item, size_t index) {
    return TemplateString::TransformTemplate(TemplateType::Value, text_, informations, item, index);
}

InformationId TemplateString::GetFirstInformationId(const Informations &informations) {
    return TemplateString::FindFirstInformationId(text_, informations);
}

} // namespace mmotd::tty_template::tty_string
