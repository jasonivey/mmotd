// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"

#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

// #include <toml/get.hpp>
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
    void SetDefault(std::string name, T value, std::string section = std::string{"core"});

    template<typename T>
    void Override(std::string name, T value, std::string section = std::string{"core"});

    bool Contains(const std::string &name) const;

    std::optional<bool> GetBoolean(const std::string &name) const noexcept;
    bool GetBoolean(const std::string &name, bool default_value) const noexcept;

    std::optional<std::int64_t> GetInteger(const std::string &name) const noexcept;
    std::int64_t GetInteger(const std::string &name, std::int64_t default_value) const noexcept;

    std::optional<double> GetDouble(const std::string &name) const noexcept;
    double GetDouble(const std::string &name, double default_value) const noexcept;

    std::optional<std::string> GetString(const std::string &name) const noexcept;
    std::string GetString(const std::string &name, std::string default_value) const noexcept;

    bool WriteDefaultConfigOptions(std::filesystem::path file_path) const;
    std::string to_string() const;

private:
    ConfigOptions();

    template<typename T>
    void Initialize(T &input);

    toml::value FindValue(std::string input_name) const;

    toml::value core_value_;
};

template<typename T>
void ConfigOptions::Override(std::string name, T value, std::string section) {
    if (!core_value_.is_table()) {
        return;
    } else if (std::empty(section)) {
        core_value_[name] = value;
    } else if (!core_value_.contains(section)) {
        core_value_[section] = toml::table{{name, value}};
    } else {
        core_value_[section][name] = value;
    }
}

template<typename T>
void ConfigOptions::SetDefault(std::string name, T value, std::string section) {
    if (!core_value_.is_table()) {
        return;
    }
    if (std::empty(section) && !core_value_.contains(name)) {
        // Set the default value iff the name is not already set
        core_value_[name] = value;
    } else if (!std::empty(section)) {
        // Set the default value iff the section exists and the name is not already set
        if (core_value_.contains(section) && core_value_[section].is_table() && !core_value_[section].contains(name)) {
            core_value_[section][name] = value;
        } else if (!core_value_.contains(section)) {
            core_value_[section] = toml::table{{name, value}};
        }
    }
}

} // namespace mmotd::core
