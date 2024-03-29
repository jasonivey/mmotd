// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/algorithm.h"
#include "common/include/logging.h"
#include "common/include/output_template.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

#include <boost/algorithm/string.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <fmt/color.h>
#include <nlohmann/json.hpp>

using nlohmann::json;
namespace fs = std::filesystem;
using fmt::format;
using namespace std;

namespace mmotd::output_template {

OutputTemplate::OutputTemplate() : template_config_(), column_items_(GetDefaultColumnItems()) {}

optional<OutputTemplate> OutputTemplate::ParseOutputTemplate(string file_name) {
    auto ec = std::error_code{};
    if (auto file_exists = fs::exists(file_name, ec); !file_exists || ec) {
        LOG_ERROR("determining whether '{}' exists", file_name);
        return nullopt;
    }
    auto input = ifstream(file_name);
    if (!input.is_open()) {
        LOG_ERROR("unable to open '{}'", file_name);
        return nullopt;
    }
    return OutputTemplate::ParseJson(input);
}

optional<OutputTemplate> OutputTemplate::ParseJson(std::istream &input) {
    auto root = json::parse(input);
    auto output_template = OutputTemplate{};
    if (!root.contains("config") || !root.at("config").is_object()) {
        LOG_ERROR("unable to deserialize the config from JSON block:\n{}", root.dump(2));
        return nullopt;
    }
    if (!root.contains("column_items") || !root.at("column_items").is_array()) {
        LOG_ERROR("unable to deserialize the column items from JSON block:\n{}", root.dump(2));
        return nullopt;
    }
    output_template.from_json(root);
    return {output_template};
}

const vector<int> &OutputTemplate::GetColumns() const {
    return template_config_.columns;
}

const TemplateColumnItems &OutputTemplate::GetColumnItems() const {
    return column_items_;
}

const mmotd::output_template::OutputSettings &OutputTemplate::GetOutputSettings() const {
    return template_config_.output_settings;
}

void OutputTemplate::from_json(const json &root) {
    PRECONDITIONS(root.contains("config") && root.at("config").is_object(),
                  "json does not contain {} or it is not an {}",
                  quoted("config"),
                  quoted("object"));
    PRECONDITIONS(root.contains("column_items") && root.at("column_items").is_array(),
                  "json does not contain {} or it is not an {}",
                  quoted("column_items"),
                  quoted("array"));

    auto template_config = root.at("config").get<TemplateConfig>();
    auto items = TemplateColumnItems{};

    for (auto &root_item : root.at("column_items")) {
        auto column_item = TemplateItemSettings{};
        column_item.from_json(root_item, &template_config.default_settings);
        if (column_item.is_valid(template_config)) {
            items.emplace_back(std::move(column_item));
        }
    }
    column_items_ = std::move(items);
    template_config_ = std::move(template_config);
}

void OutputTemplate::to_json(json &root) const {
    auto column_items = json::array();
    for (const auto &column_item : column_items_) {
        auto json_item = json();
        column_item.to_json(json_item, template_config_.default_settings);
        column_items.emplace_back(std::move(json_item));
    }
    root = json{{"config", template_config_}, {"column_items", column_items}};
}

TemplateColumnItems OutputTemplate::GetDefaultColumnItems() {
    auto items = TemplateColumnItems{};
    {
        auto item = TemplateColumnItem{};
        item.indent_size = 0;
        item.value = {
            "%ID_GENERAL_GREETING%, %ID_GENERAL_USER_NAME%! %ID_GENERAL_LOCAL_TIME_EMOJI% Welcome to %ID_SYSTEM_INFORMATION_PLATFORM_NAME% %ID_SYSTEM_INFORMATION_PLATFORM_VERSION% (%ID_SYSTEM_INFORMATION_KERNEL_TYPE% %ID_SYSTEM_INFORMATION_KERNEL_RELEASE% %ID_SYSTEM_INFORMATION_ARCHITECTURE%)"};
        item.value_color = {fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_green)};
        item.append_newlines = 2;
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.value = {"System information as of %ID_GENERAL_LOCAL_DATE_TIME%"};
        item.value_color = {fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_yellow)};
        item.append_newlines = 2;
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"%ID_WEATHER_LOCATION%:"};
        item.value = {
            "%ID_WEATHER_WEATHER%, %color:bold_bright_green%Sunrise:%color:bold_bright_white% %ID_WEATHER_SUNRISE%, %color:bold_bright_green%Sunset:%color:bold_bright_white% %ID_WEATHER_SUNSET%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"Last Login:", "", ""};
        item.name_color = {fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_cyan),
                           fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_cyan),
                           fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_cyan)};
        item.value = {"%ID_LAST_LOGIN_LOGIN_SUMMARY%",
                      "Log in: %color:bold_bright_white%%ID_LAST_LOGIN_LOGIN_TIME%",
                      "Log out: %color:bold_bright_white%%ID_LAST_LOGIN_LOGOUT_TIME%"};
        item.value_color = {fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_white),
                            fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_green),
                            fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_green)};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"Up Time:"};
        item.value = {"%ID_BOOT_TIME_UP_TIME%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.append_newlines = 2;
        item.name = {"Boot Time:"};
        item.value = {"%ID_BOOT_TIME_BOOT_TIME%"};
        items.emplace_back(item);
    }
    // Hardware specific
    {
        auto item = TemplateColumnItem{};
        item.name = {"Type:"};
        item.value = {"%ID_HARDWARE_MACHINE_TYPE%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"Model:"};
        item.value = {"%ID_HARDWARE_MACHINE_MODEL%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"CPU:"};
        item.value = {"%ID_HARDWARE_CPU_NAME%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"CPU Cores:"};
        item.value = {"%ID_HARDWARE_CPU_CORE_COUNT% (%ID_HARDWARE_CPU_BYTE_ORDER%)"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"CPU Temperature:"};
        item.value = {"%ID_HARDWARE_CPU_TEMPERATURE%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"GPU:"};
        item.value = {"%ID_HARDWARE_GPU_MODEL_NAME%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"GPU Temperature:"};
        item.value = {"%ID_HARDWARE_GPU_TEMPERATURE%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"Monitor:"};
        item.value = {"%ID_HARDWARE_MONITOR_NAME%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"Resolution:"};
        item.value = {"%ID_HARDWARE_MONITOR_RESOLUTION%"};
        item.append_newlines = 2;
        items.emplace_back(item);
    }
    // The rest of the information
    {
        auto item = TemplateColumnItem{};
        item.name = {"Computer Name:"};
        item.value = {"%ID_SYSTEM_INFORMATION_COMPUTER_NAME%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"Hostname:"};
        item.value = {"%ID_SYSTEM_INFORMATION_HOST_NAME%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"Public IP:"};
        item.value = {"%ID_EXTERNAL_NETWORK_INFO_EXTERNAL_IP%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"System Load:"};
        item.value = {"%ID_LOAD_AVERAGE_LOAD_AVERAGE%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"Processes:"};
        item.value = {"%ID_PROCESSES_PROCESS_COUNT%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"Users Logged In:"};
        item.value = {"%ID_LOGGED_IN_USER_LOGGED_IN%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"Usage of /:"};
        item.value = {"%ID_FILE_SYSTEM_SUMMARY%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"Memory Usage:"};
        item.value = {"%ID_MEMORY_USAGE_SUMMARY%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.name = {"Swap Usage:"};
        item.value = {"%ID_SWAP_USAGE_SUMMARY%"};
        items.emplace_back(item);
    }
    {
        auto item = TemplateColumnItem{};
        item.is_repeatable = true;
        item.is_optional = true;
        item.name = {"IP %ID_NETWORK_INFO_INTERFACE_NAME%:", "Mac %ID_NETWORK_INFO_INTERFACE_NAME%:"};
        item.name_color = {fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_cyan),
                           fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_cyan)};
        item.value = {"%ID_NETWORK_INFO_IP%", "%ID_NETWORK_INFO_MAC%"};
        item.value_color = {fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_white),
                            fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_white)};
        items.emplace_back(item);
    }
    // Whether the package manager is annoying us with updates
    {
        auto item = TemplateColumnItem{};
        item.indent_size = 0;
        item.is_optional = true;
        item.value_color = {fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_yellow)};
        item.value = {"%ID_PACKAGE_MANAGEMENT_UPDATE_DETAILS%"};
        item.append_newlines = 1;
        item.prepend_newlines = 1;
        items.emplace_back(item);
    }
    // Use the fortune databases to print a random "software engineering" quote
    {
        auto item = TemplateColumnItem{};
        item.indent_size = 0;
        item.is_optional = true;
        item.value_color = {fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_green)};
        item.value = {"%ID_FORTUNE_FORTUNE%"};
        item.append_newlines = 1;
        item.prepend_newlines = 1;
        items.emplace_back(item);
    }
    // Notify the user whether the package manager has a reboot required
    {
        auto item = TemplateColumnItem{};
        item.indent_size = 0;
        item.is_optional = true;
        item.value_color = {fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_red)};
        item.value = {"%ID_PACKAGE_MANAGEMENT_REBOOT_REQUIRED%"};
        item.append_newlines = 1;
        item.prepend_newlines = 1;
        items.emplace_back(item);
    }
    return items;
}

void from_json(const json &root, OutputTemplate &output_template) {
    output_template.from_json(root);
}

void to_json(json &root, const OutputTemplate &output_template) {
    output_template.to_json(root);
}

unique_ptr<OutputTemplate> MakeOutputTemplate(string file_name) {
    auto output_template_holder = OutputTemplate::ParseOutputTemplate(file_name);
    if (output_template_holder) {
        return make_unique<OutputTemplate>(move(*output_template_holder));
    }
    return nullptr;
}

unique_ptr<OutputTemplate> MakeOutputTemplateFromDefault() {
    return make_unique<OutputTemplate>();
}

bool WriteDefaultOutputTemplate(fs::path file_path) {
    auto output_template = OutputTemplate{};
    auto root = json{};
    to_json(root, output_template);

    auto output = ofstream(file_path);
    if (!output.is_open()) {
        LOG_ERROR("unable to open '{}' for writing output template", file_path);
        return false;
    }
    output << root.dump(2, ' ') << endl;
    return true;
}

} // namespace mmotd::output_template
