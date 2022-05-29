// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
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

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <toml.hpp>
#include <toml/parser.hpp>
#include <toml/types.hpp>
#include <toml/value.hpp>

using namespace std;
using namespace std::string_literals;
namespace fs = std::filesystem;
using fmt::format;

namespace {

auto GetDefaultCoreValue() -> toml::value {
    using namespace toml::literals::toml_literals;
    return u8R"(
# modified message of the day
[core]
output_color=true

# The location of the output template, "mmotd_template.json", can be
#  specified on the command line or with the following variable:
# template_path="$HOME/.config/mmotd/mmotd_template.json"

[fortune]
# fortune: the facility to print a random, hopefully interesting, adage
#  the 'fortune.db_directory' can specify a directory which contains the
#  file specified by 'fortune.db_file'.  The 'fortune.db_directory'
#  variables can be used specify a non-default directory where fortune
#  files are located. In addition, the 'fortune.db_file' variable can
#  be used to specify a non-default fortune file (whether it be a .dat)
#  database file or a text file which contains a list of fortunes seperated
#  by a line with only a '%' character.
#
# The default value for the 'fortune.db_directory' variable in macOS is:
#  db_directory="/usr/local/opt/fortune/share/games/fortunes"
# The default value for the 'fortune.db_directory' variable in Linux is:
#  db_directory="/usr/share/games/fortunes"
# The default value for 'fortune.file_name' variable is:
#  file_name="softwareengineering"

[location]
# This is the same value that is the last couple of segments of the file linked to /etc/localtime
#  i.e. /etc/localtime -> /var/db/timezone/zoneinfo/America/Denver
timezone="America/Denver"

# The following three values are used to look-up the local whether, http://wttr.in/Albuquerque%20NM%20USA
city="Albuquerque"
state="NM"
country="USA"

[logging]
# Which log level (and higher) to output:
#  trace, debug, info, warn, err, critical, off
#  The value can be specified as a string or as a number:
#  "trace" -> 0, "debug" -> 1, "info" -> 2, "warn" -> 3, "err" -> 4, "critical" -> 5, "off" -> 6
severity="trace"
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

inline mmotd::logging::Severity ConvertLoggingSeverity(const string &severity_str) {
    using mmotd::logging::Severity;
    if (boost::iequals(severity_str, "trace"s)) {
        return Severity::trace;
    } else if (boost::iequals(severity_str, "debug"s)) {
        return Severity::debug;
    } else if (boost::iequals(severity_str, "info"s)) {
        return Severity::info;
    } else if (boost::iequals(severity_str, "warn"s)) {
        return Severity::warn;
    } else if (boost::iequals(severity_str, "err"s)) {
        return Severity::err;
    } else if (boost::iequals(severity_str, "critical"s)) {
        return Severity::critical;
    } else {
        // if (boost::iequals(severity_str, "off"s))
        return Severity::off;
    }
}

inline optional<mmotd::logging::Severity> ConvertValueToLoggingSeverity(const toml::value &value) {
    if (value.is_string() && !empty(value.as_string().str)) {
        return ConvertLoggingSeverity(value.as_string().str);
    } else if (value.is_integer()) {
        auto raw_severity = std::clamp(value.as_integer(), int64_t{0}, int64_t{6});
        return static_cast<mmotd::logging::Severity>(raw_severity);
    }
    return mmotd::logging::Severity::warn;
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
    Initialize(file_path);
}

void ConfigOptions::ParseConfigFile(std::istream &input) {
    Initialize(input);
}

template<typename T>
void ConfigOptions::Initialize(T &input) {
    core_value_ = toml::parse<toml::preserve_comments>(input);
    auto output_color = special_files::IsStdoutTty();
    SetDefault("output_color"s, output_color, "core"s);
    SetDefault("file_name"s, "softwareengineering", "fortune"s);
    SetDefault("severity"s, "trace"s, "logging"s);
}

toml::value ConfigOptions::FindValue(string input_name) const {
    if (empty(input_name)) {
        return toml::value{};
    }
    auto names_deque = deque<string>{};
    boost::split(names_deque, input_name, boost::is_any_of("."));
    return FindValueImpl(core_value_, queue<string>{std::move(names_deque)});
}

optional<mmotd::logging::Severity> ConfigOptions::GetLoggingSeverity(const string &name) const {
    return ConvertValueToLoggingSeverity(FindValue(name));
}

optional<mmotd::logging::Severity> ConfigOptions::GetLoggingSeverity(const string_view &name) const {
    return GetLoggingSeverity(string(name));
}

bool ConfigOptions::Contains(const std::string &name) const {
    auto value = FindValue(name);
    return !value.is_uninitialized();
}

optional<bool> ConfigOptions::GetBoolean(const string &name) const noexcept {
    auto value = FindValue(name);
    return value.is_boolean() ? make_optional(value.as_boolean()) : nullopt;
}

bool ConfigOptions::GetBoolean(const string &name, bool default_value) const noexcept {
    auto value_holder = GetBoolean(name);
    return value_holder ? *value_holder : default_value;
}

optional<int64_t> ConfigOptions::GetInteger(const string &name) const noexcept {
    auto value = FindValue(name);
    return value.is_integer() ? make_optional(value.as_integer()) : nullopt;
}

int64_t ConfigOptions::GetInteger(const string &name, int64_t default_value) const noexcept {
    auto value_holder = GetInteger(name);
    return value_holder ? *value_holder : default_value;
}

optional<double> ConfigOptions::GetDouble(const string &name) const noexcept {
    auto value = FindValue(name);
    return value.is_floating() ? make_optional(value.as_floating()) : nullopt;
}

double ConfigOptions::GetDouble(const string &name, double default_value) const noexcept {
    auto value_holder = GetDouble(name);
    return value_holder ? *value_holder : default_value;
}

optional<string> ConfigOptions::GetString(const string &name) const noexcept {
    auto value = FindValue(name);
    if (value.is_string() && !empty(value.as_string().str)) {
        // carefull not to wrap a dissappearing object (string) -- create temporary
        // so it's lifetime will match that of the optional
        return make_optional(string{value.as_string().str});
    } else {
        return nullopt;
    }
}

string ConfigOptions::GetString(const string &name, string default_value) const noexcept {
    auto value_holder = GetString(name);
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
    return boost::trim_right_copy(toml::format(core_value_, 120u));
}

} // namespace mmotd::core
