// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/config_options.h"

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

auto GetDefaultCoreValue() -> toml::value {
    using namespace toml::literals::toml_literals;
    return u8R"(
#
# modified message of the day
#

# Enabling this may cause a performance cost. Every write to the log file will be flushed
#  when set to 'true'. This disables the natural file output caching of log file writes and
#  may enable seeing more debug statements before a crash.
logging_flush=false

# Replace the following with the location of where your "mmotd_template.json" exists
# template_path="$HOME/.config/mmotd/mmotd_template.json"

# fortune: the facility to print a random, hopefully interesting, adage
#  the 'fortune_db_path' specifies the directory which has contains the file specified
#  by 'fortune_file_name'.
#
# This is a valid example for these values on macOS after installing the 'fortune'
#  application and also installing the 'softwareengineering' fortunes.
# fortune_db_path="/usr/local/opt/fortune/share/games/fortunes"
# fortune_file_name="softwareengineering"
#
# The following empty defaults will also use the fortune installation path and the
#  'softwareengineering' fortunes.
fortune_db_path=""
fortune_file_name=""

# This is the same value that is the last couple of segments of the file linked to /etc/localtime
# /var/db/timezone/zoneinfo/America/Denver
timezone="America/Denver"

# The following three values are used to look-up the local whether, http://wttr.in/Albuquerque%20NM%20USA
city="Albuquerque"
state="NM"
country="USA"
)"_toml;
}

toml::value FindValueImpl(const toml::value &toml_value, queue<string> names) {
    const toml::value *result_ptr = &toml_value;
    while (!empty(names) && result_ptr != nullptr) {
        const auto name = names.front();
        names.pop();
        if (result_ptr->is_table() && result_ptr->contains(name)) {
            const auto &value_ref = toml::find(*result_ptr, name);
            result_ptr = &value_ref;
        } else {
            result_ptr = nullptr;
        }
    }
    return result_ptr == nullptr ? toml::value{} : *result_ptr;
}

} // namespace

namespace mmotd::core {

ConfigOptions::ConfigOptions() : core_value_(GetDefaultCoreValue()) {}

ConfigOptions &ConfigOptions::Instance(bool reinitialize) {
    static auto config_options_ptr = unique_ptr<ConfigOptions>(new ConfigOptions);
    if (reinitialize) {
        auto auto_ptr = unique_ptr<ConfigOptions>(new ConfigOptions);
        std::swap(config_options_ptr, auto_ptr);
    }
    return *config_options_ptr;
}

void ConfigOptions::ParseConfigFile(fs::path file_path) {
    core_value_ = toml::parse(file_path);
}

void ConfigOptions::ParseConfigFile(std::istream &input) {
    core_value_ = toml::parse(input);
}

toml::value ConfigOptions::FindValue(string input_name) const {
    if (empty(input_name)) {
        return toml::value{};
    }
    auto names_deque = deque<string>{};
    boost::split(names_deque, input_name, boost::is_any_of("."));
    return FindValueImpl(core_value_, queue<string>{std::move(names_deque)});
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

    if (!value.is_uninitialized() && value.is_string() && !empty(value.as_string().str)) {
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

bool ConfigOptions::WriteDefaultConfigOptions(fs::path file_path) const {
    auto output = ofstream(file_path);
    if (!output.is_open()) {
        return false;
    }
    output << to_string();
    return true;
}

string ConfigOptions::to_string() const {
    return toml::format(core_value_, 120u);
}

} // namespace mmotd::core
