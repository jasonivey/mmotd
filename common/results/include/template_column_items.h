// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"

#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/color.h>
#include <nlohmann/json_fwd.hpp>

namespace mmotd::results::color {

fmt::text_style from_color_string(std::string_view input);
std::string to_string(fmt::text_style txt_style);

} // namespace mmotd::results::color

namespace mmotd::results::data {

struct TemplateConfig;

static constexpr int ENTIRE_LINE = std::numeric_limits<int>::max() - 1;
static constexpr std::string_view ENTIRE_LINE_REPR = "ENTIRE_LINE";

struct TemplateItemSettings {
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(TemplateItemSettings);
    friend bool operator==(const TemplateItemSettings &lhs, const TemplateItemSettings &rhs) = default;

    int indent_size = 2;
    int row_index = -1;
    int repeatable_index = 0;
    int column = ENTIRE_LINE;
    int prepend_newlines = 0;
    int append_newlines = 1;
    bool is_repeatable = false;
    bool is_optional = false;
    std::vector<std::string> name;
    std::vector<fmt::text_style> name_color = {fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_cyan)};
    std::vector<std::string> value;
    std::vector<fmt::text_style> value_color = {fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_white)};

    fmt::text_style GetNameColor(size_t index = 0ull) const noexcept;
    fmt::text_style GetValueColor(size_t index = 0ull) const noexcept;

    std::string to_string() const;
    bool is_valid(const TemplateConfig &default_settings);
    bool IsEntireLine() const noexcept;

    void from_json(const nlohmann::json &root, const TemplateItemSettings *default_settings);
    void to_json(nlohmann::json &root, const TemplateItemSettings &default_settings) const;

private:
    void default_to_json(nlohmann::json &root) const;
    void not_default_to_json(nlohmann::json &root, const TemplateItemSettings &default_settings) const;
};

void from_json(const nlohmann::json &root, TemplateItemSettings &settings);

struct OutputSettings {
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(OutputSettings);

    bool collapse_column_rows = true;
    std::string table_type = "PLAIN_STYLE";

    std::string to_string() const;

    void from_json(const nlohmann::json &root);
    void to_json(nlohmann::json &root) const;
};

void from_json(const nlohmann::json &root, OutputSettings &settings);
void to_json(nlohmann::json &root, const OutputSettings &settings);

struct TemplateConfig {
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(TemplateConfig);

    std::vector<int> columns = {ENTIRE_LINE};
    TemplateItemSettings default_settings;
    OutputSettings output_settings;

    std::string to_string() const;

    void from_json(const nlohmann::json &root);
    void to_json(nlohmann::json &root) const;

    std::string columns_to_string() const;

private:
    std::string output_settings_to_string() const;
    std::string default_settings_to_string() const;

    void columns_from_json(const nlohmann::json &root);
    void default_settings_from_json(const nlohmann::json &root);
    void output_settings_from_json(const nlohmann::json &root);
};

void from_json(const nlohmann::json &root, TemplateConfig &template_config);
void to_json(nlohmann::json &root, const TemplateConfig &settings);

using TemplateColumnItem = TemplateItemSettings;
using TemplateColumnItems = std::vector<TemplateColumnItem>;

std::string to_string(const TemplateColumnItems &items);

} // namespace mmotd::results::data
