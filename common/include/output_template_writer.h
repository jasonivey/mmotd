// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/information.h"
#include "common/include/template_column_items.h"

#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

namespace mmotd::output_template_writer {

auto FindAndReplaceColorSpecifications(std::string_view input, bool test = false) -> std::string;

class OutputTemplateWriter {
    friend std::ostream &operator<<(std::ostream &os, const OutputTemplateWriter &writer);

public:
    OutputTemplateWriter(std::vector<int> column_indexes,
                         mmotd::output_template::TemplateColumnItems items,
                         mmotd::information::Informations informations);

private:
    std::vector<int> column_indexes_;
    mmotd::output_template::TemplateColumnItems items_;
    mmotd::information::Informations informations_;
};

std::string to_string(const OutputTemplateWriter &output);

} // namespace mmotd::output_template_writer
