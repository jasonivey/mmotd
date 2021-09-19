// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/config_options.h"
#include "common/include/logging.h"
#include "common/include/special_files.h"

#include <algorithm>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <istream>
#include <iterator>
#include <memory>
#include <optional>
#include <queue>
#include <system_error>
#include <variant>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <toml.hpp>
#include <toml/parser.hpp>
#include <toml/types.hpp>
#include <toml/value.hpp>

using namespace std;
namespace fs = std::filesystem;
using fmt::format;

namespace {

bool IsRequestingConfigPath(string name) {
    return name == "config_path" || boost::ends_with(name, ".config_path");
}

bool IsRequestingTemplatePath(string name) {
    return name == "template_path" || boost::ends_with(name, ".template_path");
}

toml::value FindValueInTable(const toml::value &root_value, queue<string> names) {
    MMOTD_PRECONDITIONS(!root_value.is_uninitialized(), "unable to find value when the root is uninitialized");
    MMOTD_PRECONDITIONS(size(names) >= 2, "the names queue must have atleast two values (root and value to find)");

    const toml::value *result_ptr = nullptr;
    do {
        const auto name = names.front();
        names.pop();
        if (result_ptr == nullptr) {
            result_ptr = &root_value;
        } else if (result_ptr->is_table() && result_ptr->contains(name)) {
            const auto &value_ref = toml::find(*result_ptr, name);
            result_ptr = &value_ref;
        } else {
            result_ptr = nullptr;
        }
    } while (!empty(names) && result_ptr != nullptr);

    return result_ptr == nullptr ? toml::value{} : *result_ptr;
}

toml::value FindValueImpl(const toml::value &core, const toml::value &cli, queue<string> names) {
    using mmotd::core::ConfigOptions;
    MMOTD_PRECONDITIONS(!empty(names), "the queue of names should never be empty");
    const auto &search_table = names.front() == ConfigOptions::CORE_TABLE ? core : cli;
    return FindValueInTable(search_table, names);
}

fs::path FindPath(const toml::value &root, string name) {
    using namespace mmotd::core::special_files;
    MMOTD_PRECONDITIONS(root.is_table(),
                        format(FMT_STRING("toml root value must always be a table, not {}"), root.type()));

    if (!root.contains(name) || !root.at(name).is_string()) {
        LOG_ERROR("root toml::value does not contain {} or it is not a string", quoted(name));
        return fs::path{};
    }

    const auto &path_str = root.at(name).as_string().str;
    if (empty(path_str)) {
        LOG_DEBUG("root toml::value[{}] is an empty string", quoted(name));
    } else {
        auto path = fs::path(path_str);
        auto ec = error_code{};
        if (fs::exists(path, ec) && !ec) {
            return path;
        } else {
            LOG_DEBUG("root toml::value[{}] is {}, but the file is not found", quoted(name), quoted(path.string()));
        }
    }

    return fs::path{};
}

fs::path LocateConfigPath(const toml::value &core) {
    using namespace mmotd::core::special_files;
    auto config_path = core.is_uninitialized() ? fs::path{} : FindPath(core, "config_path"s);
    if (empty(config_path)) {
        return FindFileInDefaultLocations(CONFIG_FILENAME);
    }
    return config_path;
}

fs::path LocateTemplatePath(const toml::value &cli, const toml::value &core) {
    using namespace mmotd::core::special_files;
    auto cli_template_path = cli.is_uninitialized() ? fs::path{} : FindPath(cli, "template_path"s);
    auto core_template_path = core.is_uninitialized() ? fs::path{} : FindPath(core, "template_path"s);
    if (empty(cli_template_path) && empty(core_template_path)) {
        return FindFileInDefaultLocations(TEMPLATE_FILENAME);
    } else if (!empty(cli_template_path) && !empty(core_template_path)) {
        return cli_template_path;
    } else if (!empty(cli_template_path)) {
        return cli_template_path;
    } else {
        return core_template_path;
    }
}

} // namespace

namespace mmotd::core {

ConfigOptions &ConfigOptions::Instance(bool reinitialize) {
    static auto config_options_ptr = make_unique<ConfigOptions>();
    if (reinitialize) {
        auto new_config_options_ptr = make_unique<ConfigOptions>();
        std::swap(config_options_ptr, new_config_options_ptr);
    }
    return *config_options_ptr;
}

void ConfigOptions::ParseConfigFile(fs::path file_path) {
    LOG_VERBOSE("parsing config file: {}", file_path);
    auto ec = error_code{};
    if (!fs::exists(file_path, ec) || ec) {
        MMOTD_ALWAYS_FAIL(format(FMT_STRING("config file '{}' does not exist"), file_path));
    }
    core_value_ = toml::parse(file_path);
    LOG_VERBOSE("\nconfig options:\n{}", to_string());
}

void ConfigOptions::ParseConfigFile(istream &input, const string &file_name) {
    LOG_VERBOSE("parsing config file: {}", file_name);
    core_value_ = toml::parse(input, file_name);
    LOG_VERBOSE("\nconfig options:\n{}", to_string());
}

void ConfigOptions::AddCliConfigOptions(istream &input) {
    MMOTD_CHECKS(cli_value_.is_uninitialized(), "CLI options are added once as a toml stream");
    cli_value_ = toml::parse(input, "cli-options.toml");
    InitializeConfigPath();
    if (!empty(GetConfigPath())) {
        ParseConfigFile(GetConfigPath());
    }
    InitializeTemplatePath();
    LOG_VERBOSE("\nconfig options:\n{}", to_string());
}

void ConfigOptions::InitializeConfigPath() {
    config_path_ = LocateConfigPath(cli_value_);
    LOG_VERBOSE("config path: {}", quoted(config_path_));
}

void ConfigOptions::InitializeTemplatePath() {
    template_path_ = LocateTemplatePath(cli_value_, core_value_);
    LOG_VERBOSE("template path: {}", quoted(template_path_));
}

toml::value ConfigOptions::FindValue(string input_name) const {
    if (empty(input_name)) {
        LOG_ERROR("unable to search for config option with an empty name");
        return toml::value{};
    } else if (IsRequestingConfigPath(input_name)) {
        LOG_VERBOSE("requesting config path: {}", quoted(GetConfigPath()));
        return toml::value(GetConfigPath());
    } else if (IsRequestingTemplatePath(input_name)) {
        LOG_VERBOSE("requesting template path: {}", quoted(GetTemplatePath()));
        return toml::value(GetTemplatePath());
    }

    auto names_deque = deque<string>{};
    boost::split(names_deque, input_name, boost::is_any_of("."), boost::token_compress_on);
    MMOTD_CHECKS(!empty(names_deque),
                 format(FMT_STRING("splitting {} on '.' must have at least one value"), quoted(input_name)));

    if (names_deque.front() != ConfigOptions::CORE_TABLE && names_deque.front() != ConfigOptions::CLI_TABLE) {
        names_deque.push_front(string{ConfigOptions::CORE_TABLE});
    }

    return FindValueImpl(core_value_, cli_value_, queue<string>{std::move(names_deque)});
}

bool ConfigOptions::Contains(const std::string &name) const {
    auto value = FindValue(name);
    return !value.is_uninitialized();
}

optional<bool> ConfigOptions::GetValueAsBoolean(const string &name) const noexcept {
    auto value = FindValue(name);
    return !value.is_uninitialized() && value.is_boolean() ? make_optional(bool{value.as_boolean()}) : nullopt;
}

bool ConfigOptions::GetValueAsBooleanOr(const string &name, bool default_value) const noexcept {
    auto value_holder = GetValueAsBoolean(name);
    return value_holder ? *value_holder : default_value;
}

optional<int64_t> ConfigOptions::GetValueAsInteger(const string &name) const noexcept {
    auto value = FindValue(name);
    return !value.is_uninitialized() && value.is_integer() ? make_optional(int64_t{value.as_integer()}) : nullopt;
}

int64_t ConfigOptions::GetValueAsIntegerOr(const string &name, int64_t default_value) const noexcept {
    auto value_holder = GetValueAsInteger(name);
    return value_holder ? *value_holder : default_value;
}

optional<double> ConfigOptions::GetValueAsFloating(const string &name) const noexcept {
    auto value = FindValue(name);
    return !value.is_uninitialized() && value.is_floating() ? make_optional(double{value.as_floating()}) : nullopt;
}

double ConfigOptions::GetValueAsFloatingOr(const string &name, double default_value) const noexcept {
    auto value_holder = GetValueAsFloating(name);
    return value_holder ? *value_holder : default_value;
}

optional<string> ConfigOptions::GetValueAsString(const string &name) const noexcept {
    auto value = FindValue(name);

    if (!value.is_uninitialized() && value.is_string()) {
        // carefull not to wrap a dissappearing object (string) -- create temporary
        // so it's lifetime will match that of the optional
        return make_optional(string{value.as_string().str});
    } else {
        return nullopt;
    }
}

string ConfigOptions::GetValueAsStringOr(const string &name, string default_value) const noexcept {
    auto value_holder = GetValueAsString(name);
    return value_holder ? *value_holder : default_value;
}

string ConfigOptions::to_string() const {
    auto result = string{"\n"};
    if (core_value_.is_uninitialized()) {
        result += "[core]\n<uninitialized>\n";
    } else {
        result += format(FMT_STRING("[core]\n{}"), toml::format(core_value_));
    }
    result += result.back() != '\n' ? string{"\n"} : string{};
    if (cli_value_.is_uninitialized()) {
        result += "[cli]\n<uninitialized>\n";
    } else {
        result += format(FMT_STRING("[cli]\n{}"), toml::format(cli_value_));
    }
    return result;
}

} // namespace mmotd::core
