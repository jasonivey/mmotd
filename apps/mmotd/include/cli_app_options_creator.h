// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "common/include/app_options.h"
#include "common/include/app_options_creator.h"

#include <string>

namespace CLI {
class App;
}

class CliAppOptionsCreator : public AppOptionsCreator {
public:
    ~CliAppOptionsCreator() override;

    static CliAppOptionsCreator *ParseCommandLine(const int argc, char **argv);

    const Options &GetOptions() const override { return options_; }

    bool IsAppFinished() const override { return app_finished_; }
    bool IsErrorExit() const override { return error_exit_; }

private:
    static CliAppOptionsCreator &GetInstance();

    void Parse(const int argc, char **argv);

    void AddOptionsToSubCommand(CLI::App &app);
    void AddOptionDeclarations(CLI::App &app);

    CliAppOptionsCreator() = default;

    static void WriteDefaultConfiguration(std::string file_name, std::string app_config);
    static void WriteDefaultOutputTemplate(std::string file_name);

    Options options_;
    bool app_finished_ = false;
    bool error_exit_ = false;
};
