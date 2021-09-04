// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/config_option.h"
#include "common/include/config_options.h"
#include "common/include/logging.h"

#include <algorithm>
#include <deque>
#include <filesystem>
#include <fstream>
#include <istream>
#include <memory>
#include <optional>
#include <queue>
#include <system_error>
#include <variant>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <toml.hpp>
#include <toml/parser.hpp>
#include <toml/types.hpp>
#include <toml/value.hpp>

using namespace std;
namespace fs = std::filesystem;
using fmt::format;

namespace mmotd::core {

ConfigOptions &ConfigOptions::Instance(bool reinitialize) {
    static auto config_options_ptr = make_unique<ConfigOptions>();
    if (reinitialize) {
        auto new_config_options_ptr = make_unique<ConfigOptions>();
        std::swap(config_options_ptr, new_config_options_ptr);
    }
    return *config_options_ptr;
}

void ConfigOptions::ParseConfigFile(const string &file_name) {
    auto file_path = fs::path{file_name};
    auto ec = error_code{};
    if (!fs::exists(file_path, ec) || ec) {
        MMOTD_ALWAYS_FAIL(format("config file '{}' does not exist", file_name));
    }
    core_value_ = toml::parse(file_path);
    LOG_VERBOSE("config options: {}", to_string());
}

void ConfigOptions::ParseConfigFile(istream &input, const string &file_name) {
    core_value_ = toml::parse(input, file_name);
    LOG_VERBOSE("config options: {}", to_string());
}

void ConfigOptions::AddCliConfigOptions(istream &input) {
    MMOTD_CHECKS(cli_value_.is_uninitialized(), "CLI options are added once as a toml stream");
    cli_value_ = toml::parse(input, "cli-options.toml");
}

static toml::value FindValue(const toml::value &core, const toml::value &cli, string input_name) {
    if (empty(input_name)) {
        LOG_ERROR("unable to search for config option with an empty name");
        return toml::value{};
    }
    auto names_deque = deque<string>{};
    boost::split(names_deque, input_name, boost::is_any_of("."), boost::token_compress_on);
    MMOTD_CHECKS(!empty(names_deque), "splitting for '.' must always return at least one element");

    if (names_deque.front() != ConfigOptions::CORE_TABLE && names_deque.front() != ConfigOptions::CLI_TABLE) {
        names_deque.push_front(string{ConfigOptions::CORE_TABLE});
    }
    auto names = queue<string>{std::move(names_deque)};
    const toml::value *result_ptr = nullptr;
    do {
        const auto &name = names.front();
        names.pop();
        if (result_ptr == nullptr) {
            if (name == ConfigOptions::CLI_TABLE && cli.is_table()) {
                result_ptr = &cli;
            } else if (name == ConfigOptions::CORE_TABLE && core.is_table()) {
                result_ptr = &core;
            }
        } else if (result_ptr->is_table() && result_ptr->contains(name)) {
            const auto &value_ref = toml::find(*result_ptr, name);
            result_ptr = &value_ref;
        } else {
            result_ptr = nullptr;
        }
    } while (!empty(names) && result_ptr != nullptr);

    return result_ptr != nullptr ? *result_ptr : toml::value{};
}

bool ConfigOptions::Contains(const std::string &name) const {
    auto value = FindValue(core_value_, cli_value_, name);
    return !value.is_uninitialized();
}

optional<bool> ConfigOptions::GetValueAsBoolean(const string &name) const noexcept {
    auto value = FindValue(core_value_, cli_value_, name);
    return !value.is_uninitialized() && value.is_boolean() ? make_optional(value.as_boolean()) : nullopt;
}

bool ConfigOptions::GetValueAsBooleanOr(const string &name, bool default_value) const noexcept {
    auto value_holder = GetValueAsBoolean(name);
    return value_holder ? *value_holder : default_value;
}

optional<int64_t> ConfigOptions::GetValueAsInteger(const string &name) const noexcept {
    auto value = FindValue(core_value_, cli_value_, name);
    return !value.is_uninitialized() && value.is_integer() ? make_optional(value.as_integer()) : nullopt;
}

int64_t ConfigOptions::GetValueAsIntegerOr(const string &name, int64_t default_value) const noexcept {
    auto value_holder = GetValueAsInteger(name);
    return value_holder ? *value_holder : default_value;
}

optional<double> ConfigOptions::GetValueAsFloating(const string &name) const noexcept {
    auto value = FindValue(core_value_, cli_value_, name);
    return !value.is_uninitialized() && value.is_floating() ? make_optional(value.as_floating()) : nullopt;
}

double ConfigOptions::GetValueAsFloatingOr(const string &name, double default_value) const noexcept {
    auto value_holder = GetValueAsFloating(name);
    return value_holder ? *value_holder : default_value;
}

optional<string> ConfigOptions::GetValueAsString(const string &name) const noexcept {
    auto value = FindValue(core_value_, cli_value_, name);
    return !value.is_uninitialized() && value.is_string() ? make_optional(value.as_string()) : nullopt;
}

string ConfigOptions::GetValueAsStringOr(const string &name, string default_value) const noexcept {
    auto value_holder = GetValueAsString(name);
    return value_holder ? *value_holder : default_value;
}

string ConfigOptions::to_string() const {
    auto result = string{};
    if (!core_value_.is_uninitialized()) {
        result += format(FMT_STRING("core: {}"), toml::format(core_value_));
    }
    if (!cli_value_.is_uninitialized()) {
        if (!empty(result)) {
            result += "\n";
        }
        result += format(FMT_STRING("cli: {}"), toml::format(cli_value_));
    }
    return result;
}

} // namespace mmotd::core
