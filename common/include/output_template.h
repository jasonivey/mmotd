// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"
#include "common/include/template_column_items.h"

#include <filesystem>
#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace mmotd::output_template {

class OutputTemplate {
public:
    OutputTemplate();
    NO_CONSTRUCTOR_DEFAULT_COPY_MOVE_OPERATORS_DESTRUCTOR(OutputTemplate);

    // These are the methods to parse the file when the --template arg is specified
    static std::optional<OutputTemplate> ParseOutputTemplate(std::string file_name);
    static std::optional<OutputTemplate> ParseJson(std::istream &input);

    // Get the column numbers (i.e. 0, 1, 2 or even 10, 12, 14)
    const std::vector<int> &GetColumns() const;

    // Get the items which is displayed on each row of the column
    const TemplateColumnItems &GetColumnItems() const;

    // Get the output settings
    const OutputSettings &GetOutputSettings() const;

    // Serialization to/from JSON
    void from_json(const nlohmann::json &root);
    void to_json(nlohmann::json &root) const;

private:
    static TemplateColumnItems GetDefaultColumnItems();

    TemplateConfig template_config_;
    TemplateColumnItems column_items_;
};

void from_json(const nlohmann::json &root, OutputTemplate &output_template);
void to_json(nlohmann::json &root, const OutputTemplate &output_template);

std::unique_ptr<OutputTemplate> MakeOutputTemplate(std::string file_name);
std::unique_ptr<OutputTemplate> MakeOutputTemplateFromDefault();
bool WriteDefaultOutputTemplate(std::filesystem::path file_path);

} // namespace mmotd::output_template
