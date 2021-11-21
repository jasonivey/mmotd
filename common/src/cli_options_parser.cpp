// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/algorithm.h"
#include "common/include/cli_options_parser.h"
#include "common/include/config_options.h"
#include "common/include/logging.h"
#include "common/include/special_files.h"
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
#include <string_view>
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

    bool SetConfigPath(const vector<string> &paths) { return AssignFirst(paths, config_path_, true, false); }
    bool SetTemplatePath(const vector<string> &paths) { return AssignFirst(paths, template_path_, true, false); }
    bool SetOutputConfigPath(const vector<string> &paths) {
        return AssignFirst(paths, output_config_path_, false, true);
    }
    bool SetOutputTemplatePath(const vector<string> &paths) {
        return AssignFirst(paths, output_template_path_, false, true);
    }
    bool SetLogSeverity(const vector<string> &severities) { return AssignFirst(severities, log_severity_); }
    bool SetColorWhen(const vector<string> &whens) { return AssignFirst(whens, color_when_); }

private:
    bool AssignFirst(const vector<string> &values,
                     filesystem::path &value,
                     bool must_exist = false,
                     bool must_not_exist = false);
    bool AssignFirst(const vector<string> &severities, LogSeverity &log_severity);
    bool AssignFirst(const vector<string> &whens, ColorWhen &color_when);

    filesystem::path config_path_;
    filesystem::path template_path_;
    filesystem::path output_config_path_;
    filesystem::path output_template_path_;
    LogSeverity log_severity_ = LogSeverity::NotSet;
    ColorWhen color_when_ = ColorWhen::NotSet;
};

bool CliOptions::AssignFirst(const vector<string> &values,
                             filesystem::path &value,
                             bool must_exist,
                             bool must_not_exist) {
    if (empty(values)) {
        return false;
    }
    auto path = filesystem::path{values.front()};
    if (must_exist) {
        auto ec = error_code{};
        if (!filesystem::exists(path, ec) || ec) {
            LOG_ERROR("attempting to set: '{}', which must exist and it is not found in the file system", path);
            return false;
        }
    } else if (must_not_exist) {
        auto ec = error_code{};
        if (filesystem::exists(path, ec) || ec) {
            LOG_ERROR("attempting to set: '{}', which must not exist and the file is found in the file system", path);
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

pair<bool, bool> CliOptionsParser::ParseCommandLine(const int argc, char **argv) {
    auto parser = CliOptionsParser{};
    return parser.Parse(argc, argv);
}

CliOptionsParser::CliOptionsParser() : options_(new CliOptions) {}

CliOptionsParser::~CliOptionsParser() = default;

pair<bool, bool> CliOptionsParser::Parse(const int argc, char **argv) {
    bool app_finished = true;
    bool error_exit = false;
    auto app = CLI::App{"modified message of the day", "mmotd"};
    // app.get_formatter()->column_width(40ull);
    try {
        AddOptionDeclarations(app);
        app.parse(argc, argv);
        app_finished = false;
    } catch (const CLI::CallForHelp &help) {
        const auto &msg = app.help("", CLI::AppFormatMode::All);
        LOG_INFO("{}", msg);
        cout << msg;
    } catch (const CLI::CallForVersion &version) {
        const string msg = format(FMT_STRING("version: {}"), app.version());
        LOG_INFO("{}", msg);
        cout << msg << endl;
    } catch (const CLI::ParseError &err) {
        if (err.get_exit_code() != 0) {
            LOG_FATAL("CLI error {}: {}", err.get_exit_code(), err.what());
        }
        error_exit = true;
    }
    auto output_config_path = options_->GetOutputConfigPath();
    if (!output_config_path.empty()) {
        if (!WriteDefaultConfiguration(output_config_path)) {
            LOG_FATAL("CLI error 204: unable to write default config to {}", quoted(output_config_path.string()));
            error_exit = true;
        }
        app_finished = true;
    }
    auto output_template_path = options_->GetOutputTemplatePath();
    if (!output_template_path.empty()) {
        if (!WriteDefaultTemplate(output_template_path)) {
            LOG_FATAL("CLI error 205: unable to write default template to {}", quoted(output_template_path.string()));
            error_exit = true;
        }
        app_finished = true;
    }
    if (!app_finished) {
        AddConfigOptions();
    }
    return make_pair(app_finished, error_exit);
}

void CliOptionsParser::AddConfigOptions() const {
    using mmotd::core::special_files::IsStdoutTty;
    auto config_path = options_->GetConfigPath();
    LOG_VERBOSE("using config file: {}", std::quoted(config_path.string()));

    auto template_path = options_->GetTemplatePath();
    LOG_VERBOSE("using template file: {}", std::quoted(template_path.string()));

    auto log_severity = options_->GetLogSeverity();
    if (log_severity == LogSeverity::NotSet || log_severity == LogSeverity::None) {
#if defined(DEBUG)
        log_severity = LogSeverity::Verbose;
#else
        log_severity = LogSeverity::Error;
#endif
    }

    auto color_when = options_->GetColorWhen();
    auto color_output = false;
    if (color_when == ColorWhen::NotSet || color_when == ColorWhen::Auto) {
        color_output = IsStdoutTty();
    } else {
        color_output = color_when == ColorWhen::Always;
    }

    if (!empty(config_path)) {
        ConfigOptions::Instance().ParseConfigFile(config_path);
    }
    if (!empty(template_path)) {
        ConfigOptions::Instance().AddConfigOption("template_path"s, template_path.string());
    }
    ConfigOptions::Instance().AddConfigOption("log_severity"s, static_cast<int64_t>(log_severity));
    ConfigOptions::Instance().AddConfigOption("color_output"s, color_output);
}

void CliOptionsParser::AddOptionsToSubCommand(CLI::App &app) {
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
    using namespace mmotd::core::special_files;
    // allow_windows_style_options = defaults: { windows=true / non-windows=false }
    app.option_defaults()->configurable(true)->multi_option_policy(CLI::MultiOptionPolicy::Throw);

    app.set_version_flag(
           "-V, --version",
           []() { return Version::Instance().to_string(); },
           "Prints version information")
        ->configurable(false);

    auto default_locations = GetDefaultLocationsStr();
    constexpr auto config_description_fmt =
        R"(Path to optional "toml" configuration file to specifiy application details. Default file name is {}. Default locations are {}.)";
    auto config_description = fmt::vformat(fmt::to_string_view(config_description_fmt),
                                           fmt::make_format_args(quoted(CONFIG_FILENAME), default_locations));
    app.add_option(
           "-c, --config",
           [this](auto &&paths) { return options_->SetConfigPath(forward<decltype(paths)>(paths)); },
           mmotd::algorithms::split_sentence(config_description, 70ull))
        ->required(false)
        ->check(CLI::ExistingFile)
        ->envname("MMOTD_CONFIG_PATH")
        ->configurable(false);

    constexpr auto template_description_fmt =
        R"(Path to optional "json" template file for specifying output properties. Default file name is {}. Default locations are {}.)";
    auto template_description = fmt::vformat(fmt::to_string_view(template_description_fmt),
                                             fmt::make_format_args(quoted(TEMPLATE_FILENAME), default_locations));
    app.add_option(
           "-t, --template",
           [this](auto &&paths) { return options_->SetTemplatePath(forward<decltype(paths)>(paths)); },
           mmotd::algorithms::split_sentence(template_description, 70ull))
        ->required(false)
        ->check(CLI::ExistingFile)
        ->envname("MMOTD_TEMPLATE_PATH")
        ->configurable(false);

    auto log_severity_description = "Set the log severity (none, fatal, error, warning, info, debug, verbose)";
    const LogSeverityValidator log_severity_validator;
    app.add_option(
           "-l, --log-severity,",
           [this](auto &&severities) { return options_->SetLogSeverity(forward<decltype(severities)>(severities)); },
           mmotd::algorithms::split_sentence(log_severity_description, 70ull))
        ->check(log_severity_validator)
        ->configurable(false);

    const ColorWhenValidator color_validator;
    app.add_option(
           "-p,--pretty",
           [this](auto &&whens) { return options_->SetColorWhen(forward<decltype(whens)>(whens)); },
           "When to use terminal colors (always, auto, never)")
        ->check(color_validator)
        ->configurable(false);

    AddOptionsToSubCommand(app);
}

static void PrintStatus(string_view msg, bool error = false) {
    constexpr auto error_header_ts = fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_red);
    constexpr auto header_ts = fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_green);
    constexpr auto main_ts = fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_white);
    if (error) {
        fmt::print(error_header_ts, FMT_STRING("[{:^7}]"), "ERROR");
    } else {
        fmt::print(header_ts, FMT_STRING("[{:^7}]"), "INFO");
    }
    fmt::print(main_ts, FMT_STRING(": {}\n"), msg);
}

bool CliOptionsParser::WriteDefaultConfiguration(fs::path file_path) {
    PrintStatus(format(FMT_STRING("start:  writing default configuration to '{}'"), file_path));
    auto result = ConfigOptions::Instance().WriteDefaultConfigOptions(file_path);
    PrintStatus(
        format(FMT_STRING("finish: writing default configuration to '{}', write status: {}"), file_path, result),
        result);
    return result;
}

bool CliOptionsParser::WriteDefaultTemplate(fs::path file_path) {
    using namespace mmotd::results;
    PrintStatus(format(FMT_STRING("start:  writing output template to '{}'"), file_path));
    auto result = WriteDefaultOutputTemplate(file_path);
    PrintStatus(format(FMT_STRING("finish: writing output template to '{}', write status: {}"), file_path, result),
                result);
    return result;
}

} // namespace mmotd::core
