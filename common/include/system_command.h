// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace mmotd::system::command {

std::optional<std::string> Run(const std::filesystem::path &exe_path, const std::vector<std::string> &args);

template<class Converter>
auto Run(const std::filesystem::path &exe_path, const std::vector<std::string> &args, Converter converter) {
    return converter(Run(exe_path, args));
}

} // namespace mmotd::system::command
