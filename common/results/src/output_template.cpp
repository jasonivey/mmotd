// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/algorithm.h"
#include "common/results/include/output_template.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

#include <boost/algorithm/string.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <fmt/color.h>
#include <nlohmann/json.hpp>
#include <plog/Log.h>

using nlohmann::json;
namespace fs = std::filesystem;
using fmt::format;
using namespace std;

namespace mmotd::results {

const vector<int> &OutputTemplate::GetColumns() const {
    return template_config_.columns;
}

const OutputTemplate::TemplateColumnItems &OutputTemplate::GetColumnItems() const {
    return column_items_;
}

const mmotd::results::data::OutputSettings &OutputTemplate::GetOutputSettings() const {
    return template_config_.output_settings;
}

string OutputTemplate::GetDefaultTemplate() const {
    auto root = json::object();
    auto template_config = data::TemplateConfig{};
    json config_value = json::object();
    data::to_json(config_value, template_config);
    json config = {{"config", config_value}};
    json column_items = {{"column_items", json::array()}};
    root.insert(column_items.begin(), column_items.end());
    root.insert(config.begin(), config.end());
    return root.dump(4);
}

bool OutputTemplate::ParseTemplateFile(string template_file_name) {
    auto ec = std::error_code{};
    if (auto file_exists = fs::exists(template_file_name, ec); !file_exists || ec) {
        PLOG_ERROR << format(FMT_STRING("determining whether template file {} exists"), template_file_name);
        return false;
    }
    return ParseJson(template_file_name);
}

bool OutputTemplate::ParseJson(string template_file_name) {
    auto input_stream = ifstream(template_file_name);
    auto root = json::parse(input_stream, nullptr, true, true);

    if (root.contains("config") && root.at("config").is_object()) {
        template_config_ = root.at("config").get<data::TemplateConfig>();
    }
    if (root.contains("column_items") && root.at("column_items").is_array()) {
        // unable to use column_items_ = root.get<data::TemplateItemSettings>() since implicit options need to be
        //  provided by the default_settings object (template_config_.default_settings)
        for (auto &root_item : root.at("column_items")) {
            auto column_item = data::TemplateItemSettings{};
            column_item.from_json(root_item, &template_config_.default_settings);
            if (column_item.validate(template_config_)) {
                column_items_.push_back(column_item);
            }
        }
    }
    return true;
}

unique_ptr<OutputTemplate> MakeOutputTemplate(string template_file_name) {
    auto output_template = make_unique<OutputTemplate>();
    return output_template->ParseTemplateFile(template_file_name) ? move(output_template) : nullptr;
}

void CreateDefaultOutputTemplate(string template_file_name) {
    auto ostrm = ofstream(fs::path(template_file_name));
    ostrm << OutputTemplate{}.GetDefaultTemplate();
    ostrm.flush();
}

} // namespace mmotd::results
