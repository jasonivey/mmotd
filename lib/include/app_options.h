// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>
#include <boost/logic/tribool.hpp>

class AppOptionsCreator;

struct Options {
    friend std::string to_string(const Options &options);
    friend std::ostream &operator<<(std::ostream &out, const Options &options);

    std::string to_string() const;

    int verbose = 0;
    std::optional<std::string> log_config_path;
    std::string config_path;
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

    void SetVerbose(std::int64_t count) { verbose = static_cast<int>(count); }
    bool SetLogConfigPath(const std::vector<std::string> &paths) {
        if (!paths.empty()) { log_config_path = paths.front(); }
        return true;
    }
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

    bool IsVerboseSet() const { return verbose > 0; }
    int GetVerbosityLevel() const { return verbose; }
    bool IsLogConfigPathSet() const { return static_cast<bool>(log_config_path); };
    std::string GetLogConfigPathSet() const { return log_config_path.value_or(std::string{}); };
    bool IsLastLoginSet() const { return !indeterminate(last_login); }
    bool GetLastLoginValue() const { return static_cast<bool>(last_login); }
    bool IsComputerNameSet() const { return !indeterminate(computer_name); }
    bool GetComputerNameValue() const { return static_cast<bool>(computer_name); }
    bool IsHostNameSet() const { return !indeterminate(host_name); }
    bool GetHostNameValue() const { return static_cast<bool>(host_name); }
    bool IsPublicIpSet() const { return !indeterminate(public_ip); }
    bool GetPublicIpValue() const { return static_cast<bool>(public_ip); }
    bool IsUnreadMailSet() const { return !indeterminate(unread_mail); }
    bool GetUnreadMailValue() const { return static_cast<bool>(unread_mail); }
    bool IsSystemLoadSet() const { return !indeterminate(system_load); }
    bool GetSystemLoadValue() const { return static_cast<bool>(system_load); }
    bool IsProcessorCountSet() const { return !indeterminate(processor_count); }
    bool GetProcessorCountValue() const { return static_cast<bool>(processor_count); }
    bool IsDiskUsageSet() const { return !indeterminate(disk_usage); }
    bool GetDiskUsageValue() const { return static_cast<bool>(disk_usage); }
    bool IsUsersCountSet() const { return !indeterminate(users_count); }
    bool GetUsersCountValue() const { return static_cast<bool>(users_count); }
    bool IsMemoryUsageSet() const { return !indeterminate(memory_usage); }
    bool GetMemoryUsageValue() const { return static_cast<bool>(memory_usage); }
    bool IsSwapUsageSet() const { return !indeterminate(swap_usage); }
    bool GetSwapUsageValue() const { return static_cast<bool>(swap_usage); }
    bool IsActiveNetworkInterfacesSet() const { return !indeterminate(active_network_interfaces); }
    bool GetActiveNetworkInterfacesValue() const { return static_cast<bool>(active_network_interfaces); }
    bool IsGreetingSet() const { return !indeterminate(greeting); }
    bool GetGreetingValue() const { return static_cast<bool>(greeting); }
    bool IsHeaderSet() const { return !indeterminate(header); }
    bool GetHeaderValue() const { return static_cast<bool>(header); }
    bool IsSubHeaderSet() const { return !indeterminate(sub_header); }
    bool GetSubHeaderValue() const { return static_cast<bool>(sub_header); }
    bool IsQuoteSet() const { return !indeterminate(quote); }
    bool GetQuoteValue() const { return static_cast<bool>(quote); }
};

namespace AppOption {

}

class AppOptions {
public:
    static const AppOptions *Initialize(const AppOptionsCreator &creator);

    const Options &GetOptions() const { return options_; }
    Options &GetOptions() { return options_; }

    template<typename T>
    T GetValue(const std::string &name) const;

private:
    AppOptions() = default;

    void AddOptions(const AppOptionsCreator &creator);

    Options options_;
};
