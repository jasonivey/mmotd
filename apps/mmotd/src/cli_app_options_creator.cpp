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

CliAppOptionsCreator &CliAppOptionsCreator::GetInstance() {
    static auto cli_app_options_creator = CliAppOptionsCreator{};
    return cli_app_options_creator;
}

CliAppOptionsCreator *CliAppOptionsCreator::ParseCommandLine(const int argc, char **argv) {
    auto &creator = GetInstance();
    creator.Parse(argc, argv);
    return &creator;
}

CliAppOptionsCreator::~CliAppOptionsCreator() {
}

void CliAppOptionsCreator::Parse(const int argc, char **argv) {
    app_finished_ = true;
    auto app = CLI::App{"modified message of the day"};
    try {
        AddOptionDeclarations(app);
        app.parse(argc, argv);
        app_finished_ = false;
    } catch (const CLI::CallForHelp &help) {
        const auto &msg = app.help("", CLI::AppFormatMode::All);
        MMOTD_LOG_INFO(msg);
        cout << msg << endl;
    } catch (const CLI::CallForVersion &version) {
        const string msg = format(FMT_STRING("version: {}"), app.version());
        MMOTD_LOG_INFO(msg);
        cout << msg << endl;
    } catch (const CLI::ParseError &err) {
        if (err.get_exit_code() != 0) {
            MMOTD_LOG_ERROR(format(FMT_STRING("error code {}: {}"), err.get_exit_code(), err.what()));
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
    MMOTD_LOG_DEBUG(format(FMT_STRING("Options:\n{}"), options_.to_string()));
}

void CliAppOptionsCreator::AddOptionsToSubCommand(CLI::App &app) {
    using std::placeholders::_1;

    auto *create_config = app.add_subcommand("create_config");
    create_config->option_defaults()->configurable(false);
    create_config
        ->add_option("-p,--path",
                     bind(&Options::SetOutputConfigPath, ref(options_), _1),
                     "path where a new default config file can be saved")
        ->check(CLI::NonexistentPath)
        ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
        ->configurable(false);

    auto *create_template = app.add_subcommand("create_template");
    create_template->option_defaults()->configurable(false);
    create_template
        ->add_option("-p,--path",
                     bind(&Options::SetOutputTemplatePath, ref(options_), _1),
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

    app.add_flag("-v,--verbose",
                 bind(&Options::SetVerbose, ref(options_), _1),
                 "increase output verbosity (can be specified multiple times, -vvv)")
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll)
        ->configurable(false);

    app.set_version_flag("-V,--version", bind(&Version::to_string, ref(Version::Instance())))->configurable(false);

    app.set_config("-c,--config", "", "path to config file for specifying additional options")
        ->check(CLI::ExistingFile)
        ->envname("MMOTD_CONFIG_PATH");

    app.add_option("-t,--template",
                   bind(&Options::SetTemplatePath, ref(options_), _1),
                   "path to template file for specifying output properties")
        ->check(CLI::ExistingFile)
        ->envname("MMOTD_TEMPLATE_PATH");

    const ColorWhenValidator color_validator;
    app.add_option("--color",
                   bind(&Options::SetColorWhen, ref(options_), _1),
                   "when to use terminal colors (always, auto, never)")
        ->check(color_validator)
        ->configurable(true);

    app.add_flag("--last-login,", bind(&Options::SetLastLogin, ref(options_), _1), "display last login")->group("");

    app.add_flag("--computer-name,", bind(&Options::SetComputerName, ref(options_), _1), "display computer name")
        ->group("");

    app.add_flag("--host-name", bind(&Options::SetHostName, ref(options_), _1), "display host name")->group("");

    app.add_flag("--public-ip", bind(&Options::SetPublicIp, ref(options_), _1), "display public ip address")->group("");

    app.add_flag("--unread-mail", bind(&Options::SetUnreadMail, ref(options_), _1), "display unread mail")->group("");

    app.add_flag("--system-load", bind(&Options::SetSystemLoad, ref(options_), _1), "display system load")->group("");

    app.add_flag("--processor-count", bind(&Options::SetProcessorCount, ref(options_), _1), "display processor count")
        ->group("");

    app.add_flag("--disk-usage", bind(&Options::SetDiskUsage, ref(options_), _1), "display disk usage")->group("");

    app.add_flag("--users-count", bind(&Options::SetUsersCount, ref(options_), _1), "display users logged in count")
        ->group("");

    app.add_flag("--memory-usage", bind(&Options::SetMemoryUsage, ref(options_), _1), "display memory usage")
        ->group("");

    app.add_flag("--swap-usage", bind(&Options::SetSwapUsage, ref(options_), _1), "display swap usage")->group("");

    app.add_flag("--active-network-interfaces",
                 bind(&Options::SetActiveNetworkInterfaces, ref(options_), _1),
                 "display active network interfaces (ip and mac address)")
        ->group("");

    app.add_flag("--greeting",
                 bind(&Options::SetGreeting, ref(options_), _1),
                 "display greeting using user name, OS name, release and kernel")
        ->group("");

    app.add_flag("--header",
                 bind(&Options::SetHeader, ref(options_), _1),
                 "display the system information header with date and time")
        ->group("");

    app.add_flag("--sub-header",
                 bind(&Options::SetSubHeader, ref(options_), _1),
                 "display the system information sub-header with location, weather, sunrise and sunset")
        ->group("");

    app.add_flag("--random-quote", bind(&Options::SetQuote, ref(options_), _1), "display a random quote")->group("");
}
