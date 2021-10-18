// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace mmotd::core::special_files {

std::filesystem::path FindProjectRootDirectory();
std::string GetEnvironmentValue(std::string variable_name);
std::string ExpandEnvironmentVariables(std::string input);

bool IsStdoutTty();

constexpr const std::string_view CONFIG_FILENAME = "mmotd_config.toml";
constexpr const std::string_view TEMPLATE_FILENAME = "mmotd_template.json";

std::vector<std::filesystem::path> GetDefaultLocations();
std::string GetDefaultLocationsStr();

std::filesystem::path FindFileInDefaultLocations(std::string_view file_name);

inline std::filesystem::path FindConfigInDefaultLocations() {
    return FindFileInDefaultLocations(CONFIG_FILENAME);
}

inline std::filesystem::path FindTemplateInDefaultLocations() {
    return FindFileInDefaultLocations(TEMPLATE_FILENAME);
}

} // namespace mmotd::core::special_files
