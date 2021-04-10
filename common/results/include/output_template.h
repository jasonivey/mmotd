// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"
#include "common/results/include/template_column_items.h"

#include <memory>
#include <string>
#include <vector>

namespace mmotd::results {

class OutputTemplate {
public:
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(OutputTemplate);

    std::string GetDefaultTemplate() const;
    bool ParseTemplateFile(std::string template_file_name);

    const std::vector<int> &GetColumns() const;

    using TemplateColumnItems = std::vector<mmotd::results::data::TemplateColumnItem>;
    const TemplateColumnItems &GetColumnItems() const;

    const mmotd::results::data::OutputSettings &GetOutputSettings() const;

private:
    bool ParseJson(std::string template_file_name);

    mmotd::results::data::TemplateConfig template_config_;
    mmotd::results::data::TemplateColumnItems column_items_;
};

std::unique_ptr<OutputTemplate> MakeOutputTemplate(std::string template_file_name);
void CreateDefaultOutputTemplate(std::string template_file_name);

} // namespace mmotd::results
