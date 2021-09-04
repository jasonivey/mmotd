// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/cli_options_parser.h"
#include "common/include/config_options.h"
#include "common/include/logging.h"
#include "common/include/user_information.h"
#include "common/include/version.h"
#include "common/results/include/output_template.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <istream>
#include <iterator>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <system_error>

#include <CLI/CLI.hpp>
#include <boost/algorithm/string.hpp>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <toml.hpp>

using mmotd::version::Version;
using namespace std;
using namespace std::string_literals;
using fmt::format;
namespace fs = std::filesystem;

namespace {

auto IsStdoutTtyImpl() -> bool {
    auto is_stdout_tty = isatty(STDOUT_FILENO) != 0;
    LOG_VERBOSE("stdout is{} a tty", is_stdout_tty ? "" : " not");
    return is_stdout_tty;
}

auto IsStdoutTty() -> bool {
    static const auto is_stdout_tty = IsStdoutTtyImpl();
    return is_stdout_tty;
}

fs::path FindProjectRootDirectory() {
#if defined(DEBUG)
    auto dir_path = fs::current_path();
    auto ec = error_code{};
    auto finished = false;
    while (!finished) {
        auto git_dir = dir_path / ".git";
        if (auto stat = fs::status(git_dir, ec); !ec && fs::is_directory(stat)) {
            dir_path = fs::canonical(dir_path);
            finished = true;
        } else {
            auto parent_dir = dir_path.parent_path();
            auto found_root = fs::equivalent(dir_path, parent_dir, ec);
            if (ec) {
                LOG_ERROR("filesystem error {}: {}", ec.value(), ec.message());
                dir_path = fs::path{};
                finished = true;
            } else if (found_root) {
                LOG_INFO("unable to find .git directory within {}", fs::current_path());
                dir_path = fs::path{};
                finished = true;
            } else {
                dir_path = parent_dir;
            }
        }
    }
    return dir_path;
#else
    return fs::path{};
#endif
}

fs::path FindFileInDefaultLocations(string file_name) {
    auto config_path = fs::path{};
    auto ec = error_code{};

    // if running a debug build, try using the in-source version
    if (auto project_root = FindProjectRootDirectory(); !project_root.empty()) {
        config_path = project_root / "config" / file_name;
        if (fs::exists(config_path, ec) && !ec) {
            return fs::canonical(config_path);
        }
    }

    auto user_info = mmotd::core::GetUserInformation();
    if (!empty(user_info.home_directory) && fs::exists(user_info.home_directory, ec) && !ec) {
        // check for location in $XDG_CONFIG_HOME
        config_path = fs::canonical(fs::path(user_info.home_directory)) / ".config" / "mmotd" / file_name;
        if (fs::exists(config_path, ec) && !ec) {
            return fs::canonical(config_path);
        }
        // lastly, check whether it is in the root of the home directory
        config_path = fs::canonical(fs::path(user_info.home_directory)) / file_name;
        if (fs::exists(config_path, ec) && !ec) {
            return fs::canonical(config_path);
        }
    }
    return fs::path{};
}

fs::path FindConfigInDefaultLocations() {
    return FindFileInDefaultLocations("mmotd.toml");
}

fs::path FindTemplateInDefaultLocations() {
    return FindFileInDefaultLocations("mmotd_template.json");
}

} // namespace

namespace mmotd::core {

enum class ColorWhen { Always, Auto, Never, NotSet };
enum class LogSeverity : int64_t {
    None = 0,
    Fatal = 1,
    Error = 2,
    Warning = 3,
    Info = 4,
    Debug = 5,
    Verbose = 6,
    NotSet
};

class CliOptions {
public:
    CliOptions() = default;
    ~CliOptions() = default;
    CliOptions(CliOptions const &other) = delete;
    CliOptions &operator=(CliOptions const &other) = delete;
    CliOptions(CliOptions &&other) = delete;
    CliOptions &operator=(CliOptions &&other) = delete;

    filesystem::path GetConfigPath() const { return config_path_; }
    filesystem::path GetTemplatePath() const { return template_path_; }
    filesystem::path GetOutputConfigPath() const { return output_config_path_; }
    filesystem::path GetOutputTemplatePath() const { return output_template_path_; }
    LogSeverity GetLogSeverity() const { return log_severity_; }
    ColorWhen GetColorWhen() const { return color_when_; }

    bool SetConfigPath(const vector<string> &paths) { return AssignFirst(paths, config_path_, true); }
    bool SetTemplatePath(const vector<string> &paths) { return AssignFirst(paths, template_path_, true); }
    bool SetOutputConfigPath(const vector<string> &paths) { return AssignFirst(paths, output_config_path_); }
    bool SetOutputTemplatePath(const vector<string> &paths) { return AssignFirst(paths, output_template_path_); }
    bool SetLogSeverity(const vector<string> &severities) { return AssignFirst(severities, log_severity_); }
    bool SetColorWhen(const vector<string> &whens) { return AssignFirst(whens, color_when_); }

private:
    bool AssignFirst(const vector<string> &values, filesystem::path &value, bool should_exist = false);
    bool AssignFirst(const vector<string> &severities, LogSeverity &log_severity);
    bool AssignFirst(const vector<string> &whens, ColorWhen &color_when);

    filesystem::path config_path_;
    filesystem::path template_path_;
    filesystem::path output_config_path_;
    filesystem::path output_template_path_;
    LogSeverity log_severity_ = LogSeverity::NotSet;
    ColorWhen color_when_ = ColorWhen::NotSet;
};

bool CliOptions::AssignFirst(const vector<string> &values, filesystem::path &value, bool should_exist) {
    if (empty(values)) {
        return false;
    }
    auto path = filesystem::path{values.front()};
    if (should_exist) {
        auto ec = error_code{};
        if (!filesystem::exists(path, ec) || ec) {
            return false;
        }
    }
    value = path;
    return true;
}

bool CliOptions::AssignFirst(const vector<string> &severities, LogSeverity &log_severity) {
    using boost::iequals;
    if (empty(severities)) {
        return false;
    }
    auto severity = severities.front();
    if (iequals(severity, "None")) {
        log_severity = LogSeverity::None;
    } else if (iequals(severity, "Fatal")) {
        log_severity = LogSeverity::Fatal;
    } else if (iequals(severity, "Error")) {
        log_severity = LogSeverity::Error;
    } else if (iequals(severity, "Warning")) {
        log_severity = LogSeverity::Warning;
    } else if (iequals(severity, "Info")) {
        log_severity = LogSeverity::Info;
    } else if (iequals(severity, "Debug")) {
        log_severity = LogSeverity::Debug;
    } else if (iequals(severity, "Verbose")) {
        log_severity = LogSeverity::Verbose;
    } else {
        return false;
    }
    return true;
}

bool CliOptions::AssignFirst(const vector<string> &whens, ColorWhen &color_when) {
    using boost::iequals;
    if (empty(whens)) {
        return false;
    }
    auto when = whens.front();
    if (iequals(when, "Always")) {
        color_when = ColorWhen::Always;
    } else if (iequals(when, "Auto")) {
        color_when = ColorWhen::Auto;
    } else if (iequals(when, "Never")) {
        color_when = ColorWhen::Never;
    } else {
        return false;
    }
    return true;
}

class ColorWhenValidator : public CLI::Validator {
public:
    ColorWhenValidator() : Validator("WHEN") {
        using namespace boost;
        func_ = [](string &when) {
            if (!iequals(when, "Always") && !iequals(when, "Auto") && !iequals(when, "Never")) {
                return format(FMT_STRING("--color '{}' is invalid (should be 'always', 'auto' or 'never'"), when);
            }
            return string();
        };
    }
};

class LogSeverityValidator : public CLI::Validator {
public:
    LogSeverityValidator() : Validator("SEVERITY") {
        using namespace boost;
        func_ = [](string &severity) {
            if (!iequals(severity, "None") && !iequals(severity, "Fatal") && !iequals(severity, "Error") &&
                !iequals(severity, "Warning") && !iequals(severity, "Info") && !iequals(severity, "Debug") &&
                !iequals(severity, "Verbose")) {
                return format(
                    FMT_STRING(
                        "--log-severity '{}' is invalid (should be 'none', 'fatal', 'error', 'warning', 'info', 'debug' or 'verbose'"),
                    severity);
            }
            return string();
        };
    }
};

CliOptionsParser &CliOptionsParser::GetInstance() {
    static auto cli_app_options_creator = CliOptionsParser{};
    return cli_app_options_creator;
}

CliOptionsParser *CliOptionsParser::ParseCommandLine(const int argc, char **argv) {
    auto &creator = GetInstance();
    creator.Parse(argc, argv);
    return &creator;
}

CliOptionsParser::CliOptionsParser() : options_(new CliOptions) {
}

CliOptionsParser::~CliOptionsParser() = default;

void CliOptionsParser::Parse(const int argc, char **argv) {
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
    auto output_config_path = options_->GetOutputConfigPath();
    if (!output_config_path.empty()) {
        // fix_jasoni_config
        // WriteDefaultConfiguration(*output_config_path, app.config_to_str(true, true));
        app_finished_ = true;
    }
    auto output_template_path = options_->GetOutputTemplatePath();
    if (!output_template_path.empty()) {
        // fix_jasoni_config
        // WriteDefaultOutputTemplate(*output_template_path);
        app_finished_ = true;
    }
    if (!app_finished_) {
        AddConfigOptions();
    }
    // fix_jasoni_config
    // LOG_DEBUG("Options:\n{}", options_->to_string());
}

void CliOptionsParser::AddConfigOptions() const {
    auto cli_options = string{};

    auto config_path = options_->GetConfigPath();
    if (config_path.empty()) {
        config_path = FindConfigInDefaultLocations();
    }
    LOG_VERBOSE("using config file: {}", std::quoted(config_path.string()));
    cli_options += format(FMT_STRING("config_path={}\n"), std::quoted(config_path.string()));

    auto template_path = options_->GetTemplatePath();
    if (template_path.empty()) {
        template_path = FindTemplateInDefaultLocations();
    }
    LOG_VERBOSE("using template file: {}", std::quoted(template_path.string()));
    cli_options += format(FMT_STRING("template_path={}\n"), std::quoted(template_path.string()));

    auto log_severity = options_->GetLogSeverity();
    if (log_severity == LogSeverity::NotSet || log_severity == LogSeverity::None) {
#if defined(DEBUG)
        log_severity = LogSeverity::Verbose;
#else
        log_severity = LogSeverity::Error;
#endif
    }
    cli_options += format(FMT_STRING("log_severity={}\n"), static_cast<int64_t>(log_severity));

    auto color_when = options_->GetColorWhen();
    auto color_output = false;
    if (color_when == ColorWhen::NotSet || color_when == ColorWhen::Auto) {
        color_output = IsStdoutTty();
    } else {
        color_output = color_when == ColorWhen::Always;
    }
    cli_options += format(FMT_STRING("color_output={}\n"), color_output);

    auto cli_options_stream = istringstream{cli_options};
    ConfigOptions::Instance().AddCliConfigOptions(cli_options_stream);
}

void CliOptionsParser::AddOptionsToSubCommand(CLI::App &app) {
    using std::placeholders::_1;

    auto *create_config = app.add_subcommand("create_config");
    create_config->option_defaults()->configurable(false);
    create_config
        ->add_option(
            "-p,--path",
            [this](auto &&paths) { return options_->SetOutputConfigPath(forward<decltype(paths)>(paths)); },
            "path where a new default config file can be saved")
        ->check(CLI::NonexistentPath)
        ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
        ->configurable(false);

    auto *create_template = app.add_subcommand("create_template");
    create_template->option_defaults()->configurable(false);
    create_template
        ->add_option(
            "-p,--path",
            [this](auto &&paths) { return options_->SetOutputTemplatePath(forward<decltype(paths)>(paths)); },
            "path where a new default template file can be saved")
        ->check(CLI::NonexistentPath)
        ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
        ->configurable(false);
}

void CliOptionsParser::AddOptionDeclarations(CLI::App &app) {
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
           [this](auto &&paths) { return options_->SetTemplatePath(forward<decltype(paths)>(paths)); },
           "path to template file for specifying output properties")
        ->check(CLI::ExistingFile)
        ->envname("MMOTD_TEMPLATE_PATH");

    auto log_severity_description = "set the log severity (none, fatal, error, warning, info, debug, verbose)";
    const LogSeverityValidator log_severity_validator;
    app.add_option(
           "--log-severity,",
           [this](auto &&severities) { return options_->SetLogSeverity(forward<decltype(severities)>(severities)); },
           log_severity_description)
        ->check(log_severity_validator)
        ->configurable(true)
        ->group("");

    const ColorWhenValidator color_validator;
    app.add_option(
           "--color",
           [this](auto &&whens) { return options_->SetColorWhen(forward<decltype(whens)>(whens)); },
           "when to use terminal colors (always, auto, never)")
        ->check(color_validator)
        ->configurable(true);
}

static void PrintStatus(string_view msg) {
    constexpr auto header_ts = fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_green);
    constexpr auto main_ts = fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_white);
    fmt::print(header_ts, FMT_STRING("[{:^7}]"), "INFO");
    fmt::print(main_ts, FMT_STRING(": {}\n"), msg);
}

void CliOptionsParser::WriteDefaultConfiguration(string file_name, string app_config) {
    auto new_config = ofstream(file_name);
    PrintStatus(format(FMT_STRING("start:  writing default configuration to '{}'"), file_name));
    new_config << app_config << endl;
    new_config.close();
    PrintStatus(format(FMT_STRING("finish: writing default configuration to '{}'"), file_name));
}

void CliOptionsParser::WriteDefaultOutputTemplate(string file_name) {
    PrintStatus(format(FMT_STRING("start:  writing output template to '{}'"), file_name));
    mmotd::results::WriteDefaultOutputTemplate(file_name);
    PrintStatus(format(FMT_STRING("finish: writing output template to '{}'"), file_name));
}

} // namespace mmotd::core
