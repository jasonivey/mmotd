// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/app_options.h"
#include "lib/include/app_options_creator.h"

#include <fmt/format.h>
#include <iostream>
#include <string_view>

using fmt::format;
using namespace std;

std::string to_string(const Options &options) {
    return options.to_string();
}

std::ostream &operator<<(std::ostream &out, const Options &options) {
    out << options.to_string() << endl;
    return out;
}

template<typename T, typename U>
void append_option(string &existing_options_str, const string &name, T is_set, U get_value) {
    if (!existing_options_str.empty()) {
        existing_options_str += "\n";
    }
    existing_options_str += format("  {} [{}]: {}", name, is_set() ? "SET" : "UNSET", get_value());
}

std::string Options::to_string() const {
    auto options_str = string{};
    append_option(options_str, "verbose", bind(&Options::IsVerboseSet, this), bind(&Options::GetVerbosityLevel, this));
    append_option(options_str,
                  "log_config_path",
                  bind(&Options::IsLogConfigPathSet, this),
                  bind(&Options::GetLogConfigPathSet, this));
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

const AppOptions *AppOptions::Initialize(const AppOptionsCreator &creator) {
    static auto app_options = AppOptions{};
    app_options.AddOptions(creator);
    return &app_options;
}

void AppOptions::AddOptions(const AppOptionsCreator &creator) {
    options_ = creator.GetOptions();
}
