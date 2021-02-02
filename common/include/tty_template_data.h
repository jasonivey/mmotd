// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"

#include <memory>
#include <string>
#include <vector>

#include <fmt/color.h>
#include <nlohmann/json_fwd.hpp>

namespace mmotd::tty_template::color {

fmt::text_style from_color_string(std::string input);
std::string to_string(fmt::text_style txt_style);

} // namespace mmotd::tty_template::color

namespace mmotd::tty_template::data {

struct TemplateConfig;

struct TemplateItemSettings {
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(TemplateItemSettings);

    int indent_size = 2;
    int row_index = -1;
    int repeatable_index = 0;
    int column = 0;
    int prepend_newlines = 0;
    int append_newlines = 1;
    bool is_repeatable = false;
    bool is_optional = false;
    std::string name_width = "max";
    std::vector<std::string> name;
    fmt::text_style name_color = fmt::text_style{fmt::emphasis::bold} | fmt::fg(fmt::terminal_color::bright_cyan);
    std::vector<std::string> value;
    fmt::text_style value_color = fmt::text_style{fmt::emphasis::bold} | fmt::fg(fmt::terminal_color::bright_white);

    std::string to_string() const;
    bool validate(const TemplateConfig &default_settings);
};

void from_json(const nlohmann::json &root,
               TemplateItemSettings &settings,
               const TemplateItemSettings *default_settings);
void from_json(const nlohmann::json &root, TemplateItemSettings &settings);
void to_json(nlohmann::json &root, const TemplateItemSettings &settings);

struct TemplateConfig {
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(TemplateConfig);

    std::vector<int> columns = {0};
    TemplateItemSettings default_settings;

    std::string columns_to_string() const;
    std::string default_settings_to_string() const;
    std::string to_string() const;
};

void from_json(const nlohmann::json &root, TemplateConfig &template_config);
void to_json(nlohmann::json &root, const TemplateConfig &settings);

using TemplateColumnItem = TemplateItemSettings;
using TemplateColumnItems = std::vector<TemplateColumnItem>;

std::string to_string(const TemplateColumnItems &items);
// void from_json(const nlohmann::json &root, TemplateColumnItems &template_column_items);
// void to_json(nlohmann::json &root, const TemplateColumnItems &settings);

} // namespace mmotd::tty_template::data
