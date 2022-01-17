// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/config_options.h"
#include "common/include/special_files.h"
#include "common/test/include/exception_matcher.h"

#include <istream>
#include <sstream>
#include <string>
#include <string_view>
#include <stdexcept>

#include <boost/algorithm/string/split.hpp>
#include <catch2/catch.hpp>
#include <fmt/format.h>
#include <toml.hpp>

using namespace Catch;
using namespace Catch::Matchers;
using namespace std;
using namespace std::string_literals;
namespace fs = std::filesystem;
using namespace toml::literals::toml_literals;

namespace {

fs::path FindDefaultConfigPath() {
    auto project_root = mmotd::core::special_files::FindProjectRootDirectory();
    if (empty(project_root)) {
        return fs::path{};
    }
    auto ec = error_code{};
    auto template_path = project_root / "config" / "mmotd_config.toml";
    if (fs::exists(template_path, ec) && !ec) {
        return template_path;
    } else {
        return fs::path{};
    }
}

} // namespace

namespace mmotd::test {

CATCH_TEST_CASE("discover toml api", "[config options]") {
    const toml::value toml_value = R"(
    title = "this is TOML literal"
    [table]
    key = "value")"_toml;

    CATCH_CHECK(toml_value.is_table());
    CATCH_CHECK(toml_value.contains("title"));

    CATCH_CHECK(toml_value.at("table").is_table());

    auto tbl = toml_value.at("table").as_table();
    CATCH_CHECK(tbl.contains("key"));
    CATCH_CHECK(tbl.at("key").is_string());
    CATCH_CHECK(tbl.at("key").as_string() == string{"value"});
    CATCH_CHECK(toml_value.at("table").at("key").as_string() == string{"value"});
    CATCH_CHECK(toml_value.at("table").as_table().at("key").as_string() == string{"value"});
}

CATCH_TEST_CASE("ConfigOptions test parsing stream", "[config options]") {
    using mmotd::core::ConfigOptions;
    static const char *const buffer = R"(
    title = "this is TOML literal"
    color = true)";
    auto input_stream = istringstream{buffer};

    auto &config_options = ConfigOptions::Instance(true);
    config_options.ParseConfigFile(input_stream);

    CATCH_CHECK(config_options.GetString("title"sv) == "this is TOML literal");
    CATCH_CHECK(config_options.GetBoolean("color"sv));
}

CATCH_TEST_CASE("ConfigOptions test with two tables", "[config options]") {
    using mmotd::core::ConfigOptions;
    static const char *const buffer = R"(
version = "0.99.88-alpha"
nums  = [3, 1, 4, 1, 5]
[sub1]
title = "this is TOML literal"
color = true
)";
    auto input_stream = istringstream{buffer};

    auto &config_options = ConfigOptions::Instance(true);
    config_options.ParseConfigFile(input_stream);

    CATCH_CHECK(config_options.Contains("version"sv));
    CATCH_CHECK(config_options.GetString("version"sv, ""sv) == "0.99.88-alpha");
    // CHECK(config_options.GetValueAt<int64_t>("nums", 0).value_or(-1) == 3);
    // CHECK(config_options.GetValueAt<int64_t>("nums", 1).value_or(-1) == 1);
    // CHECK(config_options.GetValueAt<int64_t>("nums", 2).value_or(-1) == 4);
    // CHECK(config_options.GetValueAt<int64_t>("nums", 3).value_or(-1) == 1);
    // CHECK(config_options.GetValueAt<int64_t>("nums", 4).value_or(-1) == 5);
    CATCH_CHECK(config_options.GetString("sub1.title"sv, ""sv) == "this is TOML literal");
    CATCH_CHECK(config_options.GetBoolean("sub1.color"sv, false));
}

CATCH_TEST_CASE("ConfigOptions test with empty string", "[config options]") {
    using mmotd::core::ConfigOptions;
    static const char *const buffer = R"(
version = "0.99.88-alpha"
nums  = [3, 1, 4, 1, 5]
[sub1]
title = ""
color = true
)";
    auto input_stream = istringstream{buffer};

    auto &config_options = ConfigOptions::Instance(true);
    config_options.ParseConfigFile(input_stream);

    CATCH_CHECK(config_options.GetString("sub1.title"sv, "config value doesn't have a value"sv) ==
                "config value doesn't have a value");
}

CATCH_TEST_CASE("default ConfigOptions match", "[config options]") {
    using mmotd::core::ConfigOptions;
    const toml::value config_value1 = u8R"(
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
    auto &config_options = ConfigOptions::Instance(true);
    auto config_options_str = config_options.to_string();
    auto input_stream = istringstream{config_options_str};
    const toml::value config_value2 = toml::parse(input_stream);

    CATCH_CHECK(config_value1 == config_value2);
}

CATCH_TEST_CASE("default ConfigOptions matches mmotd_config.toml", "[OutputTemplate]") {
    using mmotd::core::ConfigOptions;

    auto default_config_path = FindDefaultConfigPath();
    CATCH_CHECK(!default_config_path.empty());

    auto default_config_stream = ifstream(default_config_path);
    CATCH_CHECK(default_config_stream.is_open());
    auto default_config_toml = toml::parse(default_config_stream);

    auto &config_options = ConfigOptions::Instance(true);
    auto config_options_str = config_options.to_string();
    auto config_options_stream = istringstream{config_options_str};
    auto config_options_toml = toml::parse(config_options_stream);

    CATCH_CHECK(default_config_toml == config_options_toml);
}

} // namespace mmotd::test
