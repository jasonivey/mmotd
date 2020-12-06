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
    Options() = default;

    std::string to_string() const;

    int verbose = 0;
    std::optional<std::string> output_config_path;
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
    bool SetOutputConfigPath(const std::vector<std::string> &paths) {
        if (!paths.empty()) {
            output_config_path = paths.front();
        }
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
};

class AppOptions {
public:
    static const AppOptions &Instance();
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
