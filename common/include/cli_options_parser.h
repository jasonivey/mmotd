// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/config_options.h"

#include <filesystem>
#include <memory>
#include <string>
#include <utility>

namespace CLI {
class App;
}

namespace mmotd::core {

class CliOptions;

class CliOptionsParser {
public:
    ~CliOptionsParser();
    CliOptionsParser(CliOptionsParser const &other) = delete;
    CliOptionsParser &operator=(CliOptionsParser const &other) = delete;
    CliOptionsParser(CliOptionsParser &&other) = delete;
    CliOptionsParser &operator=(CliOptionsParser &&other) = delete;

    static std::pair<bool, bool> ParseCommandLine(const int argc, char **argv);

private:
    CliOptionsParser();

    std::pair<bool, bool> Parse(const int argc, char **argv);

    void AddConfigOptions() const;
    void AddOptionsToSubCommand(CLI::App &app);
    void AddOptionDeclarations(CLI::App &app);

    static void WriteDefaultConfiguration(std::filesystem::path file_path, std::string app_config);
    static void WriteDefaultTemplate(std::filesystem::path file_path);

    std::unique_ptr<CliOptions> options_;
};

} // namespace mmotd::core
