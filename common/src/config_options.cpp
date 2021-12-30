// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/config_options.h"
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
# Which log level (and below) to output:
#  none, fatal, error, warning, info, debug, verbose
#  The value can be specified as a string or as a number:
#  "none" -> 0, "fatal" -> 1, "error" -> 2, "warning" -> 3, "info" -> 4, "debug" -> 5, "verbose" -> 6
severity="verbose"
# Control whether to output a colorized log file. This looks strange when
#  opening the log file in a text editor, but it works well with the following
#  command:
#  cat /tmp/mmotd-00007ccf.log | less -RFX
output_color=true
# This is mainly used for debugging. This option will flush the contents of the
#  log file after every line has been written. This provides the opportunity to
#  see the live status of the application at any given moment. (Especially
#  handy when debugging an application crash.) As a result though, enabling
#  this option may have performance reprocussions.
flush_on_write=false
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

inline bool IsLoggingSeverity(const string &name) {
    return boost::iequals(name, "logging.severity"s);
}

inline int64_t ConvertLoggingSeverity(const string &severity_str) {
    if (boost::iequals(severity_str, "fatal"s)) {
        return int64_t{1};
    } else if (boost::iequals(severity_str, "error"s)) {
        return int64_t{2};
    } else if (boost::iequals(severity_str, "warning"s)) {
        return int64_t{3};
    } else if (boost::iequals(severity_str, "info"s)) {
        return int64_t{4};
    } else if (boost::iequals(severity_str, "debug"s)) {
        return int64_t{5};
    } else if (boost::iequals(severity_str, "verbose"s)) {
        return int64_t{6};
    } else {
        // boost::iequals(severity_str, "none"s)
        return int64_t{0};
    }
}

inline int64_t GetLoggingSeverity(const toml::value &value) {
    if (value.is_string() && !empty(value.as_string().str)) {
        return ConvertLoggingSeverity(value.as_string().str);
    } else if (value.is_integer()) {
        return std::clamp(value.as_integer(), int64_t{0}, int64_t{6});
    }
    return int64_t{0};
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
    SetDefault("severity"s, "verbose"s, "logging"s);
    SetDefault("output_color"s, output_color, "logging"s);
    SetDefault("flush_on_write"s, false, "logging"s);
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
    if (IsLoggingSeverity(name)) {
        return make_optional(GetLoggingSeverity(value));
    }
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
