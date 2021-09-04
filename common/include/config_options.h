#pragma once
#include "common/include/big_five_macros.h"
#include "common/include/config_option.h"

#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <toml/value.hpp>

namespace mmotd::core {

class CliOptionsParser;

class ConfigOptions {
    friend CliOptionsParser;

public:
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(ConfigOptions);

    static ConfigOptions &Instance(bool reinitialize = false);
    void ParseConfigFile(const std::string &file_name);
    void ParseConfigFile(std::istream &input, const std::string &file_name);
    void AddCliConfigOptions(std::istream &input);

    bool Contains(const std::string &name) const;

    std::optional<bool> GetValueAsBoolean(const std::string &name) const noexcept;
    bool GetValueAsBooleanOr(const std::string &name, bool default_value) const noexcept;

    std::optional<std::int64_t> GetValueAsInteger(const std::string &name) const noexcept;
    std::int64_t GetValueAsIntegerOr(const std::string &name, std::int64_t default_value) const noexcept;

    std::optional<double> GetValueAsFloating(const std::string &name) const noexcept;
    double GetValueAsFloatingOr(const std::string &name, double default_value) const noexcept;

    std::optional<std::string> GetValueAsString(const std::string &name) const noexcept;
    std::string GetValueAsStringOr(const std::string &name, std::string default_value) const noexcept;

    std::string to_string() const;

    static constexpr std::string_view CORE_TABLE = "core";
    static constexpr std::string_view CLI_TABLE = "cli";

private:
    toml::value core_value_;
    toml::value cli_value_;
};

} // namespace mmotd::core
