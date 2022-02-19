// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

namespace mmotd::results {
class OutputTemplate;
}

namespace mmotd::information {
class Informations;
}

namespace mmotd::output {

// auto FindColorSpecifications(std::string_view input) -> ColorSpecifications;
// auto ReplaceColorSpecifications(const ColorSpecifications &color_specs, bool test = false) -> std::string;
auto FindAndReplaceColorSpecifications(std::string_view input, bool test = false) -> std::string;
// auto ReplaceEmbeddedColorCodes(mmotd::results::data::TemplateColumnItems items)
//     -> mmotd::results::data::TemplateColumnItems;
// auto ReplaceInformationIds(mmotd::results::data::TemplateColumnItems items,
//                            const mmotd::information::Informations &informations)
//     -> mmotd::results::data::TemplateColumnItems;

class Output {
public:
    // Output(std::string output) : output_(std::move(output)) {}
    // Output(const std::string_view &output) : output_(output) {}

    std::string to_string(const mmotd::results::OutputTemplate &output_template,
                          const mmotd::information::Informations &informations) const;
    // friend std::ostream &operator<<(std::ostream &os, const Output &output);

private:
    // std::string output_;
};

} // namespace mmotd::output
