#include "common/assertion/include/assertion.h"
#include "common/include/config_options.h"
#include "common/test/include/exception_matcher.h"

#include <istream>
#include <sstream>
#include <stdexcept>

#include <boost/algorithm/string/split.hpp>
#include <catch2/catch.hpp>
#include <fmt/format.h>
#include <toml.hpp>

using namespace Catch;
using namespace Catch::Matchers;
using namespace std;
using namespace toml::literals::toml_literals;

namespace mmotd::test {

TEST_CASE("discover toml api", "[config options]") {
    const toml::value toml_value = R"(
    title = "this is TOML literal"
    [table]
    key = "value")"_toml;

    CHECK(toml_value.is_table());
    CHECK(toml_value.contains("title"));

    CHECK(toml_value.at("table").is_table());

    auto tbl = toml_value.at("table").as_table();
    CHECK(tbl.contains("key"));
    CHECK(tbl.at("key").is_string());
    CHECK(tbl.at("key").as_string() == string{"value"});
    CHECK(toml_value.at("table").at("key").as_string() == string{"value"});
    CHECK(toml_value.at("table").as_table().at("key").as_string() == string{"value"});
}

TEST_CASE("ConfigOptions test parsing stream", "[config options]") {
    using mmotd::core::ConfigOptions;
    static const char *const buffer = R"(
    title = "this is TOML literal"
    color = true)";
    auto input_stream = istringstream{buffer};

    auto &config_options = ConfigOptions::Instance(true);
    config_options.ParseConfigFile(input_stream, "test.toml");

    CHECK(config_options.GetValueAsString("title") == "this is TOML literal");
    CHECK(config_options.GetValueAsBoolean("color"));
}

TEST_CASE("ConfigOptions test with two tables", "[config options]") {
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
    config_options.ParseConfigFile(input_stream, "test.toml");

    CHECK(config_options.Contains("version"));
    CHECK(config_options.GetValueAsStringOr("version", "") == "0.99.88-alpha");
    // CHECK(config_options.GetValueAt<int64_t>("nums", 0).value_or(-1) == 3);
    // CHECK(config_options.GetValueAt<int64_t>("nums", 1).value_or(-1) == 1);
    // CHECK(config_options.GetValueAt<int64_t>("nums", 2).value_or(-1) == 4);
    // CHECK(config_options.GetValueAt<int64_t>("nums", 3).value_or(-1) == 1);
    // CHECK(config_options.GetValueAt<int64_t>("nums", 4).value_or(-1) == 5);
    CHECK(config_options.GetValueAsStringOr("sub1.title", "") == "this is TOML literal");
    CHECK(config_options.GetValueAsBooleanOr("sub1.color", false));
}

TEST_CASE("ConfigOptions test with empty string", "[config options]") {
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
    config_options.ParseConfigFile(input_stream, "test.toml");

    CHECK(config_options.GetValueAsString("sub1.title") == "");
}

} // namespace mmotd::test
