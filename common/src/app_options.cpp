// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/algorithm.h"
#include "common/include/app_options.h"
#include "common/include/app_options_creator.h"

#include <filesystem>
#include <string_view>
#include <system_error>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

#include <unistd.h>

using fmt::format;
using namespace std;

namespace {

optional<string> GetEnvironmentVariableValue(string variable_name) {
    auto env_value = getenv(variable_name.c_str());
    if (env_value == nullptr) {
        PLOG_ERROR << format(FMT_STRING("getenv failed when attempting to look up variable {}"), variable_name);
        return nullopt;
    }
    return make_optional(string{env_value});
}

auto GetDefaultTemplatePath() {
    namespace fs = std::filesystem;
    auto home_dir_holder = GetEnvironmentVariableValue("HOME");
    if (!home_dir_holder) {
        return fs::path{};
    }
    auto template_path = fs::path{*home_dir_holder} / ".config" / "mmotd" / "template.json";
    auto ec = error_code{};
    if (fs::exists(template_path, ec) && !ec &&
        ((fs::is_regular_file(template_path, ec) && !ec) || (fs::is_symlink(template_path, ec) && !ec))) {
        PLOG_VERBOSE << format(FMT_STRING("found the default mmotd template ({})"), template_path.string());
        return template_path;
    } else {
        PLOG_ERROR << format(FMT_STRING("unable to locate the default mmotd template ({})"), template_path.string());
        return fs::path{};
    }
}

template<typename T, typename U>
void append_option(string &existing_options_str, const string &name, T is_set, U get_value) {
    if (!existing_options_str.empty()) {
        existing_options_str += "\n";
    }
    existing_options_str += format(FMT_STRING("  {} [{}]: {}"), name, is_set() ? "SET" : "UNSET", get_value());
}

auto IsStdoutTtyImpl() {
    auto is_stdout_tty = isatty(STDOUT_FILENO) != 0;
    PLOG_VERBOSE << format(FMT_STRING("stdout is{} a tty"), is_stdout_tty ? "" : " not");
    return is_stdout_tty;
}

auto IsStdoutTty() {
    static const auto is_stdout_tty = IsStdoutTtyImpl();
    return is_stdout_tty;
}

} // namespace

std::string Options::to_string() const {
    auto options_str = string{};
    append_option(options_str, "verbose", bind(&Options::IsVerboseSet, this), bind(&Options::GetVerbosityLevel, this));
    append_option(options_str, "color", bind(&Options::IsColorWhenSet, this), bind(&Options::GetColorWhen, this));
    append_option(options_str,
                  "template",
                  bind(&Options::IsTemplatePathSet, this),
                  bind(&Options::GetTemplatePath, this));
    append_option(options_str,
                  "last_login",
                  bind(&Options::IsLastLoginSet, this),
                  bind(&Options::GetLastLoginValue, this));
    append_option(options_str,
                  "computer_name",
                  bind(&Options::IsComputerNameSet, this),
                  bind(&Options::GetComputerNameValue, this));
    append_option(options_str,
                  "host_name",
                  bind(&Options::IsHostNameSet, this),
                  bind(&Options::GetHostNameValue, this));
    append_option(options_str,
                  "public_ip",
                  bind(&Options::IsPublicIpSet, this),
                  bind(&Options::GetPublicIpValue, this));
    append_option(options_str,
                  "unread_mail",
                  bind(&Options::IsUnreadMailSet, this),
                  bind(&Options::GetUnreadMailValue, this));
    append_option(options_str,
                  "system_load",
                  bind(&Options::IsSystemLoadSet, this),
                  bind(&Options::GetSystemLoadValue, this));
    append_option(options_str,
                  "processor_count",
                  bind(&Options::IsProcessorCountSet, this),
                  bind(&Options::GetProcessorCountValue, this));
    append_option(options_str,
                  "disk_usage",
                  bind(&Options::IsDiskUsageSet, this),
                  bind(&Options::GetDiskUsageValue, this));
    append_option(options_str,
                  "users_count",
                  bind(&Options::IsUsersCountSet, this),
                  bind(&Options::GetUsersCountValue, this));
    append_option(options_str,
                  "memory_usage",
                  bind(&Options::IsMemoryUsageSet, this),
                  bind(&Options::GetMemoryUsageValue, this));
    append_option(options_str,
                  "swap_usage",
                  bind(&Options::IsSwapUsageSet, this),
                  bind(&Options::GetSwapUsageValue, this));
    append_option(options_str,
                  "active_network_interfaces",
                  bind(&Options::IsActiveNetworkInterfacesSet, this),
                  bind(&Options::GetActiveNetworkInterfacesValue, this));
    append_option(options_str, "greeting", bind(&Options::IsGreetingSet, this), bind(&Options::GetGreetingValue, this));
    append_option(options_str, "header", bind(&Options::IsHeaderSet, this), bind(&Options::GetHeaderValue, this));
    append_option(options_str,
                  "sub_header",
                  bind(&Options::IsSubHeaderSet, this),
                  bind(&Options::GetSubHeaderValue, this));
    append_option(options_str, "quote", bind(&Options::IsQuoteSet, this), bind(&Options::GetQuoteValue, this));
    return options_str;
}

void Options::SetVerbose(std::int64_t level) noexcept {
    using mmotd::algorithms::value_in_range;
    if (value_in_range(level, static_cast<int64_t>(Verbosity::Off), static_cast<int64_t>(Verbosity::Inavlid))) {
        verbose = static_cast<Verbosity>(level);
    } else if (level < 0) {
        verbose = Verbosity::Off;
    } else {
        verbose = Verbosity::Verbose;
    }
}

bool Options::IsVerboseSet() const noexcept {
    return verbose != Verbosity::Inavlid;
}

bool Options::IsVerbosityEnabled() const noexcept {
    return verbose != Verbosity::Inavlid && verbose != Verbosity::Off;
}

Options::Verbosity Options::GetVerbosityLevel() const noexcept {
    return verbose == Verbosity::Inavlid ? Verbosity::Off : verbose;
}

bool Options::SetColorWhen(const std::vector<std::string> &whens) {
    auto when = empty(whens) ? string{} : whens.front();
    if (boost::iequals(when, "Always")) {
        color_when = ColorWhen::Always;
    } else if (boost::iequals(when, "Auto")) {
        color_when = ColorWhen::Auto;
    } else if (boost::iequals(when, "Never")) {
        color_when = ColorWhen::Never;
    }
    return true;
}

bool Options::IsColorWhenSet() const {
    return color_when != ColorWhen::NotSet;
}

Options::ColorWhen Options::GetColorWhen() const {
    return color_when == ColorWhen::NotSet ? ColorWhen::Auto : color_when;
}

bool Options::IsColorDisabled() const {
    if (color_when == ColorWhen::NotSet || color_when == ColorWhen::Auto) {
        return !IsStdoutTty();
    } else {
        return color_when == ColorWhen::Never;
    }
}

bool Options::SetOutputConfigPath(const std::vector<std::string> &paths) {
    output_config_path = empty(paths) ? string{} : paths.front();
    return true;
}

optional<string> Options::GetOutputConfigPath() const {
    return output_config_path;
}

bool Options::SetOutputTemplatePath(const std::vector<std::string> &paths) {
    output_template_path = empty(paths) ? string{} : paths.front();
    return true;
}

optional<string> Options::GetOutputTemplatePath() const {
    return output_template_path;
}

bool Options::SetTemplatePath(const std::vector<std::string> &paths) {
    template_path = empty(paths) ? string{} : paths.front();
    return true;
}

AppOptions &AppOptions::CreateInstance() {
    static auto app_options = AppOptions{};
    return app_options;
}

const AppOptions &AppOptions::Instance() {
    return CreateInstance();
}

const AppOptions *AppOptions::Initialize(const AppOptionsCreator &creator) {
    auto &app_options = AppOptions::CreateInstance();
    app_options.AddOptions(creator);

    auto &options = app_options.GetOptions();
    if (!options.IsTemplatePathSet()) {
        auto template_path = GetDefaultTemplatePath();
        if (!template_path.empty()) {
            options.SetTemplatePath(vector<string>{1, template_path.string()});
        }
    }
    return &app_options;
}

void AppOptions::AddOptions(const AppOptionsCreator &creator) {
    options_ = creator.GetOptions();
}
