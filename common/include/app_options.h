// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/algorithm.h"
#include "common/include/logging.h"

#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

#include <boost/logic/tribool.hpp>

class AppOptionsCreator;

class Options {
public:
    Options() = default;

    std::string to_string() const;

    enum class ColorWhen { Always, Auto, Never, NotSet };
    enum class LogSeverity { None = 0, Fatal = 1, Error = 2, Warning = 3, Info = 4, Debug = 5, Verbose = 6, NotSet };

    bool IsLogSeveritySet() const noexcept;
    LogSeverity GetLogSeverity() const noexcept;
    mmotd::logging::Severity GetLoggingSeverity() const noexcept;
    bool IsColorWhenSet() const;
    bool IsColorDisabled() const;
    ColorWhen GetColorWhen() const;
    bool IsTemplatePathSet() const { return !std::empty(template_path); }
    std::string GetTemplatePath() const { return template_path; }
    bool IsLastLoginSet() const { return !indeterminate(last_login); }
    bool GetLastLoginValue() const { return last_login.value; }
    bool IsComputerNameSet() const { return !indeterminate(computer_name); }
    bool GetComputerNameValue() const { return computer_name.value; }
    bool IsHostNameSet() const { return !indeterminate(host_name); }
    bool GetHostNameValue() const { return host_name.value; }
    bool IsPublicIpSet() const { return !indeterminate(public_ip); }
    bool GetPublicIpValue() const { return public_ip.value; }
    bool IsUnreadMailSet() const { return !indeterminate(unread_mail); }
    bool GetUnreadMailValue() const { return unread_mail.value; }
    bool IsSystemLoadSet() const { return !indeterminate(system_load); }
    bool GetSystemLoadValue() const { return system_load.value; }
    bool IsProcessorCountSet() const { return !indeterminate(processor_count); }
    bool GetProcessorCountValue() const { return processor_count.value; }
    bool IsDiskUsageSet() const { return !indeterminate(disk_usage); }
    bool GetDiskUsageValue() const { return disk_usage.value; }
    bool IsUsersCountSet() const { return !indeterminate(users_count); }
    bool GetUsersCountValue() const { return users_count.value; }
    bool IsMemoryUsageSet() const { return !indeterminate(memory_usage); }
    bool GetMemoryUsageValue() const { return memory_usage.value; }
    bool IsSwapUsageSet() const { return !indeterminate(swap_usage); }
    bool GetSwapUsageValue() const { return swap_usage.value; }
    bool IsActiveNetworkInterfacesSet() const { return !indeterminate(active_network_interfaces); }
    bool GetActiveNetworkInterfacesValue() const { return active_network_interfaces.value; }
    bool IsGreetingSet() const { return !indeterminate(greeting); }
    bool GetGreetingValue() const { return greeting.value; }
    bool IsHeaderSet() const { return !indeterminate(header); }
    bool GetHeaderValue() const { return header.value; }
    bool IsSubHeaderSet() const { return !indeterminate(sub_header); }
    bool GetSubHeaderValue() const { return sub_header.value; }
    bool IsQuoteSet() const { return !indeterminate(quote); }
    bool GetQuoteValue() const { return quote.value; }

    bool SetLogSeverity(const std::vector<std::string> &severities);
    bool SetColorWhen(const std::vector<std::string> &whens);
    bool SetTemplatePath(const std::vector<std::string> &paths);
    void SetLastLogin(int value) { last_login = value == -1 ? false : true; }
    void SetComputerName(int value) { computer_name = value == -1 ? false : true; }
    void SetHostName(int value) { host_name = value == -1 ? false : true; }
    void SetPublicIp(int value) { public_ip = value == -1 ? false : true; }
    void SetUnreadMail(int value) { unread_mail = value == -1 ? false : true; }
    void SetSystemLoad(int value) { system_load = value == -1 ? false : true; }
    void SetProcessorCount(int value) { processor_count = value == -1 ? false : true; }
    void SetDiskUsage(int value) { disk_usage = value == -1 ? false : true; }
    void SetUsersCount(int value) { users_count = value == -1 ? false : true; }
    void SetMemoryUsage(int value) { memory_usage = value == -1 ? false : true; }
    void SetSwapUsage(int value) { swap_usage = value == -1 ? false : true; }
    void SetActiveNetworkInterfaces(int value) { active_network_interfaces = value == -1 ? false : true; }
    void SetGreeting(int value) { greeting = value == -1 ? false : true; }
    void SetHeader(int value) { header = value == -1 ? false : true; }
    void SetSubHeader(int value) { sub_header = value == -1 ? false : true; }
    void SetQuote(int value) { quote = value == -1 ? false : true; }

    bool SetOutputConfigPath(const std::vector<std::string> &paths);
    std::optional<std::string> GetOutputConfigPath() const;
    bool SetOutputTemplatePath(const std::vector<std::string> &paths);
    std::optional<std::string> GetOutputTemplatePath() const;

private:
    LogSeverity log_severity = LogSeverity::NotSet;
    ColorWhen color_when = ColorWhen::NotSet;
    std::string config_path;
    std::string template_path;
    boost::tribool last_login = boost::indeterminate;
    boost::tribool computer_name = boost::indeterminate;
    boost::tribool host_name = boost::indeterminate;
    boost::tribool public_ip = boost::indeterminate;
    boost::tribool unread_mail = boost::indeterminate;
    boost::tribool system_load = boost::indeterminate;
    boost::tribool processor_count = boost::indeterminate;
    boost::tribool disk_usage = boost::indeterminate;
    boost::tribool users_count = boost::indeterminate;
    boost::tribool memory_usage = boost::indeterminate;
    boost::tribool swap_usage = boost::indeterminate;
    boost::tribool active_network_interfaces = boost::indeterminate;
    boost::tribool greeting = boost::indeterminate;
    boost::tribool header = boost::indeterminate;
    boost::tribool sub_header = boost::indeterminate;
    boost::tribool quote = boost::indeterminate;

    std::optional<std::string> output_config_path;
    std::optional<std::string> output_template_path;
};

class AppOptions {
public:
    static const AppOptions &Instance();
    static const AppOptions *Initialize(const AppOptionsCreator &creator);

    const Options &GetOptions() const { return options_; }
    Options &GetOptions() { return options_; }

private:
    static AppOptions &CreateInstance();
    AppOptions() = default;

    void AddOptions(const AppOptionsCreator &creator);

    Options options_;
};
