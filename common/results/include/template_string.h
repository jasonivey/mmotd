// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"

#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <tuple>
#include <vector>

#include <fmt/color.h>

namespace mmotd::information {

enum class InformationId : size_t;
class Information;
class Informations;

} // namespace mmotd::information

namespace mmotd::results::data {

struct TemplateItemSettings;
using TemplateColumnItem = TemplateItemSettings;

} // namespace mmotd::results::data

namespace mmotd::results::test {

class TemplateStringTest;

}

namespace mmotd::results {

class TemplateSubstring;
using TemplateSubstringPtr = std::unique_ptr<TemplateSubstring>;
class TemplateSubstrings;
class SubstringRange;

class TemplateString {
    friend class mmotd::results::test::TemplateStringTest;

public:
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(TemplateString);
    TemplateString(std::string text);

    std::string TransformTemplateName(const mmotd::information::Informations &informations,
                                      const mmotd::results::data::TemplateColumnItem &item,
                                      size_t information_index);
    std::string TransformTemplateValue(const mmotd::information::Informations &informations,
                                       const mmotd::results::data::TemplateColumnItem &item,
                                       size_t information_index);

    mmotd::information::InformationId GetFirstInformationId(const mmotd::information::Informations &informations);

private:
    enum class TemplateType { Name, Value };
    static std::string TransformTemplate(TemplateType template_type,
                                         const std::string &text,
                                         const mmotd::information::Informations &informations,
                                         const mmotd::results::data::TemplateColumnItem &item,
                                         size_t index);

    static std::optional<mmotd::information::Information>
    FindInformation(const std::string &information_id,
                    const mmotd::information::Informations &informations,
                    size_t information_index);

    static std::optional<std::string> GetInformationValue(const std::string &information_id,
                                                          const mmotd::information::Informations &informations,
                                                          size_t information_index);

    static std::string ReplaceInformationIds(const std::string &text,
                                             const mmotd::information::Informations &informations,
                                             size_t information_index);
    static mmotd::information::InformationId
    FindFirstInformationId(const std::string &text, const mmotd::information::Informations &informations);

    static std::vector<std::string> SplitColorCodeDefinitions(const std::string &color_str, const char delimeter = '%');
    static std::vector<std::string>
    SplitMultipleColorCodeDefinitions(const std::vector<std::string> &combined_color_defs, const char delimeter = ':');
    static std::vector<std::string> ParseColorCodeString(const std::string &color_str);

    void static CreateTemplateSubstring(const std::string &text,
                                        TemplateSubstrings &template_substrings,
                                        const SubstringRange &substring_text,
                                        const std::vector<std::string> &color_definitions,
                                        const SubstringRange &prefix);
    static void UpdateLastTemplateSubstring(const std::string &text,
                                            TemplateSubstrings &template_substrings,
                                            size_t next_template_substring_position);

    static bool IsColorCodeMatchValid(const std::string &text, const std::smatch &matches);
    static void
    ParseColorCodeSubstring(const std::string &text, size_t &index, TemplateSubstrings &template_substrings);

    static TemplateSubstrings GenerateTemplateSubstrings(const std::string &text);

    static fmt::text_style GetColorValue(std::string color_specification);
    static std::string ReplaceEmbeddedColorCodes(TemplateType template_type,
                                                 const std::string &text,
                                                 const mmotd::results::data::TemplateColumnItem &item);

    std::string text_;
};

} // namespace mmotd::results
