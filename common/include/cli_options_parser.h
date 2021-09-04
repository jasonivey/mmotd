// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "common/include/config_option.h"
#include "common/include/config_options.h"

#include <memory>
#include <string>

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

    static CliOptionsParser *ParseCommandLine(const int argc, char **argv);

    bool IsAppFinished() const { return app_finished_; }
    bool IsErrorExit() const { return error_exit_; }

private:
    CliOptionsParser();

    static CliOptionsParser &GetInstance();

    void Parse(const int argc, char **argv);

    void AddConfigOptions() const;
    void AddOptionsToSubCommand(CLI::App &app);
    void AddOptionDeclarations(CLI::App &app);

    static void WriteDefaultConfiguration(std::string file_name, std::string app_config);
    static void WriteDefaultOutputTemplate(std::string file_name);

    std::unique_ptr<CliOptions> options_;
    bool app_finished_ = false;
    bool error_exit_ = false;
};

} // namespace mmotd::core
