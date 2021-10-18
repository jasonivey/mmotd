// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"

#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>

#include <toml/value.hpp>

namespace mmotd::core {

class ConfigOptions {
public:
    NO_CONSTRUCTOR_DEFAULT_COPY_MOVE_OPERATORS_DESTRUCTOR(ConfigOptions);

    // The `reinitialize = true` parameter is only to be used by the unit-tests
    static ConfigOptions &Instance(bool reinitialize = false);

    void ParseConfigFile(std::filesystem::path file_path);

    // This method is only to be used by the unit-tests
    void ParseConfigFile(std::istream &input);

    template<typename T>
    void AddConfigOption(std::string name, T value);

    bool Contains(const std::string &name) const;

    std::optional<bool> GetValueAsBoolean(const std::string &name) const noexcept;
    bool GetValueAsBooleanOr(const std::string &name, bool default_value) const noexcept;

    std::optional<std::int64_t> GetValueAsInteger(const std::string &name) const noexcept;
    std::int64_t GetValueAsIntegerOr(const std::string &name, std::int64_t default_value) const noexcept;

    std::optional<double> GetValueAsFloating(const std::string &name) const noexcept;
    double GetValueAsFloatingOr(const std::string &name, double default_value) const noexcept;

    std::optional<std::string> GetValueAsString(const std::string &name) const noexcept;
    std::string GetValueAsStringOr(const std::string &name, std::string default_value) const noexcept;

    bool WriteDefaultConfigOptions(std::filesystem::path file_path) const;
    std::string to_string() const;

private:
    ConfigOptions();

    toml::value FindValue(std::string input_name) const;

    toml::value core_value_;
};

template<typename T>
void ConfigOptions::AddConfigOption(std::string name, T value) {
    if (core_value_.is_table()) {
        core_value_[name] = value;
    }
}

} // namespace mmotd::core
