// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "apps/mmotd/include/cli_app_options_creator.h"
#include "common/include/app_options.h"
#include "common/include/logging.h"
#include "common/include/version.h"
#include "common/results/include/output_template.h"

#include <algorithm>
#include <any>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>

#include <CLI/CLI.hpp>
#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

using mmotd::version::Version;
using namespace std;
using fmt::format;

class ColorWhenValidator : public CLI::Validator {
public:
    ColorWhenValidator() : Validator("WHEN") {
        using namespace boost;
        func_ = [](std::string &when) {
            if (!iequals(when, "Always") && !iequals(when, "Auto") && !iequals(when, "Never")) {
                return format(FMT_STRING("--color '{}' is invalid (should be 'always', 'auto' or 'never'"), when);
            }
            return std::string();
        };
    }
};

class LogSeverityValidator : public CLI::Validator {
public:
    LogSeverityValidator() : Validator("SEVERITY") {
        using namespace boost;
        func_ = [](std::string &severity) {
            if (!iequals(severity, "None") && !iequals(severity, "Fatal") && !iequals(severity, "Error") &&
                !iequals(severity, "Warning") && !iequals(severity, "Info") && !iequals(severity, "Debug") &&
                !iequals(severity, "Verbose")) {
                return format(
                    FMT_STRING(
                        "--log-severity '{}' is invalid (should be 'none', 'fatal', 'error', 'warning', 'info', 'debug' or 'verbose'"),
                    severity);
            }
            return std::string();
        };
    }
};

CliAppOptionsCreator &CliAppOptionsCreator::GetInstance() {
    static auto cli_app_options_creator = CliAppOptionsCreator{};
    return cli_app_options_creator;
}

CliAppOptionsCreator *CliAppOptionsCreator::ParseCommandLine(const int argc, char **argv) {
    auto &creator = GetInstance();
    creator.Parse(argc, argv);
    return &creator;
}

CliAppOptionsCreator::~CliAppOptionsCreator() = default;

void CliAppOptionsCreator::Parse(const int argc, char **argv) {
    app_finished_ = true;
    auto app = CLI::App{"modified message of the day"};
    try {
        AddOptionDeclarations(app);
        app.parse(argc, argv);
        app_finished_ = false;
    } catch (const CLI::CallForHelp &help) {
        const auto &msg = app.help("", CLI::AppFormatMode::All);
        LOG_INFO("{}", msg);
        cout << msg << endl;
    } catch (const CLI::CallForVersion &version) {
        const string msg = format(FMT_STRING("version: {}"), app.version());
        LOG_INFO("{}", msg);
        cout << msg << endl;
    } catch (const CLI::ParseError &err) {
        if (err.get_exit_code() != 0) {
            LOG_ERROR("error code {}: {}", err.get_exit_code(), err.what());
        }
        error_exit_ = true;
    }
    auto output_config_path = options_.GetOutputConfigPath();
    if (output_config_path) {
        auto new_config = ofstream(*output_config_path);
        new_config << app.config_to_str(true, true);
        app_finished_ = true;
    }
    auto output_template_path = options_.GetOutputTemplatePath();
    if (output_template_path) {
        using mmotd::results::CreateDefaultOutputTemplate;
        CreateDefaultOutputTemplate(*output_template_path);
        app_finished_ = true;
    }
    LOG_DEBUG("Options:\n{}", options_.to_string());
}

void CliAppOptionsCreator::AddOptionsToSubCommand(CLI::App &app) {
    using std::placeholders::_1;

    auto *create_config = app.add_subcommand("create_config");
    create_config->option_defaults()->configurable(false);
    create_config
        ->add_option(
            "-p,--path",
            [this](auto &&paths) { return options_.SetOutputConfigPath(forward<decltype(paths)>(paths)); },
            "path where a new default config file can be saved")
        ->check(CLI::NonexistentPath)
        ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
        ->configurable(false);

    auto *create_template = app.add_subcommand("create_template");
    create_template->option_defaults()->configurable(false);
    create_template
        ->add_option(
            "-p,--path",
            [this](auto &&paths) { return options_.SetOutputTemplatePath(forward<decltype(paths)>(paths)); },
            "path where a new default template file can be saved")
        ->check(CLI::NonexistentPath)
        ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
        ->configurable(false);
}

void CliAppOptionsCreator::AddOptionDeclarations(CLI::App &app) {
    using std::placeholders::_1;

    AddOptionsToSubCommand(app);

    // allow_windows_style_options = defaults: { windows=true / non-windows=false }
    app.option_defaults()->configurable(true)->multi_option_policy(CLI::MultiOptionPolicy::Throw);

    app.set_version_flag("-V,--version", []() { return Version::Instance().to_string(); })->configurable(false);

    app.set_config("-c,--config", "", "path to config file for specifying additional options")
        ->check(CLI::ExistingFile)
        ->envname("MMOTD_CONFIG_PATH");

    app.add_option(
           "-t,--template",
           [this](auto &&paths) { return options_.SetTemplatePath(forward<decltype(paths)>(paths)); },
           "path to template file for specifying output properties")
        ->check(CLI::ExistingFile)
        ->envname("MMOTD_TEMPLATE_PATH");

    auto log_severity_description = "set the log severity (none, fatal, error, warning, info, debug, verbose)";
    const LogSeverityValidator log_severity_validator;
    app.add_option(
           "--log-severity,",
           [this](auto &&severities) { return options_.SetLogSeverity(forward<decltype(severities)>(severities)); },
           log_severity_description)
        ->check(log_severity_validator)
        ->configurable(true)
        ->group("");

    const ColorWhenValidator color_validator;
    app.add_option(
           "--color",
           [this](auto &&whens) { return options_.SetColorWhen(forward<decltype(whens)>(whens)); },
           "when to use terminal colors (always, auto, never)")
        ->check(color_validator)
        ->configurable(true);

    app.add_flag(
           "--last-login,",
           [this](auto &&value) { return options_.SetLastLogin(forward<decltype(value)>(value)); },
           "display last login")
        ->group("");

    app.add_flag(
           "--computer-name,",
           [this](auto &&value) { return options_.SetComputerName(forward<decltype(value)>(value)); },
           "display computer name")
        ->group("");

    app.add_flag(
           "--host-name",
           [this](auto &&value) { return options_.SetHostName(forward<decltype(value)>(value)); },
           "display host name")
        ->group("");

    app.add_flag(
           "--public-ip",
           [this](auto &&value) { return options_.SetPublicIp(forward<decltype(value)>(value)); },
           "display public ip address")
        ->group("");

    app.add_flag(
           "--unread-mail",
           [this](auto &&value) { return options_.SetUnreadMail(forward<decltype(value)>(value)); },
           "display unread mail")
        ->group("");

    app.add_flag(
           "--system-load",
           [this](auto &&value) { return options_.SetSystemLoad(forward<decltype(value)>(value)); },
           "display system load")
        ->group("");

    app.add_flag(
           "--processor-count",
           [this](auto &&value) { return options_.SetProcessorCount(forward<decltype(value)>(value)); },
           "display processor count")
        ->group("");

    app.add_flag(
           "--disk-usage",
           [this](auto &&value) { return options_.SetDiskUsage(forward<decltype(value)>(value)); },
           "display disk usage")
        ->group("");

    app.add_flag(
           "--users-count",
           [this](auto &&value) { return options_.SetUsersCount(forward<decltype(value)>(value)); },
           "display users logged in count")
        ->group("");

    app.add_flag(
           "--memory-usage",
           [this](auto &&value) { return options_.SetMemoryUsage(forward<decltype(value)>(value)); },
           "display memory usage")
        ->group("");

    app.add_flag(
           "--swap-usage",
           [this](auto &&value) { return options_.SetSwapUsage(forward<decltype(value)>(value)); },
           "display swap usage")
        ->group("");

    app.add_flag(
           "--active-network-interfaces",
           [this](auto &&value) { return options_.SetActiveNetworkInterfaces(forward<decltype(value)>(value)); },
           "display active network interfaces (ip and mac address)")
        ->group("");

    app.add_flag(
           "--greeting",
           [this](auto &&value) { return options_.SetGreeting(forward<decltype(value)>(value)); },
           "display greeting using user name, OS name, release and kernel")
        ->group("");

    app.add_flag(
           "--header",
           [this](auto &&value) { return options_.SetHeader(forward<decltype(value)>(value)); },
           "display the system information header with date and time")
        ->group("");

    app.add_flag(
           "--sub-header",
           [this](auto &&value) { return options_.SetSubHeader(forward<decltype(value)>(value)); },
           "display the system information sub-header with location, weather, sunrise and sunset")
        ->group("");

    app.add_flag(
           "--random-quote",
           [this](auto &&value) { return options_.SetQuote(forward<decltype(value)>(value)); },
           "display a random quote")
        ->group("");
}
