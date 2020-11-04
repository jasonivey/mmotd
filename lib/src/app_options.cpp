// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/app_options.h"
#include "lib/include/app_options_creator.h"

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <fmt/format.h>
#include <string_view>
#include <iostream>

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
    append_option(options_str, "log_config_path", bind(&Options::IsLogConfigPathSet, this), bind(&Options::GetLogConfigPathSet, this));
    append_option(options_str, "last_login", bind(&Options::IsLastLoginSet, this), bind(&Options::GetLastLoginValue, this));
    append_option(options_str, "computer_name", bind(&Options::IsComputerNameSet, this), bind(&Options::GetComputerNameValue, this));
    append_option(options_str, "host_name", bind(&Options::IsHostNameSet, this), bind(&Options::GetHostNameValue, this));
    append_option(options_str, "public_ip", bind(&Options::IsPublicIpSet, this), bind(&Options::GetPublicIpValue, this));
    append_option(options_str, "unread_mail", bind(&Options::IsUnreadMailSet, this), bind(&Options::GetUnreadMailValue, this));
    append_option(options_str, "system_load", bind(&Options::IsSystemLoadSet, this), bind(&Options::GetSystemLoadValue, this));
    append_option(options_str, "processor_count", bind(&Options::IsProcessorCountSet, this), bind(&Options::GetProcessorCountValue, this));
    append_option(options_str, "disk_usage", bind(&Options::IsDiskUsageSet, this), bind(&Options::GetDiskUsageValue, this));
    append_option(options_str, "users_count", bind(&Options::IsUsersCountSet, this), bind(&Options::GetUsersCountValue, this));
    append_option(options_str, "memory_usage", bind(&Options::IsMemoryUsageSet, this), bind(&Options::GetMemoryUsageValue, this));
    append_option(options_str, "swap_usage", bind(&Options::IsSwapUsageSet, this), bind(&Options::GetSwapUsageValue, this));
    append_option(options_str, "active_network_interfaces", bind(&Options::IsActiveNetworkInterfacesSet, this), bind(&Options::GetActiveNetworkInterfacesValue, this));
    append_option(options_str, "greeting", bind(&Options::IsGreetingSet, this), bind(&Options::GetGreetingValue, this));
    append_option(options_str, "header", bind(&Options::IsHeaderSet, this), bind(&Options::GetHeaderValue, this));
    append_option(options_str, "sub_header", bind(&Options::IsSubHeaderSet, this), bind(&Options::GetSubHeaderValue, this));
    append_option(options_str, "quote", bind(&Options::IsQuoteSet, this), bind(&Options::GetQuoteValue, this));
    return options_str;
}

#ifdef _USING_BOOST_PROGRAM_OPTIONS
static vector<size_t> InitializeTags() {
    return {typeid(char).hash_code(),
            typeid(int8_t).hash_code(),
            typeid(uint8_t).hash_code(),
            typeid(int16_t).hash_code(),
            typeid(uint16_t).hash_code(),
            typeid(int32_t).hash_code(),
            typeid(uint32_t).hash_code(),
            typeid(int64_t).hash_code(),
            typeid(uint64_t).hash_code(),
            typeid(float).hash_code(),
            typeid(double).hash_code(),
            typeid(string).hash_code(),
            typeid(string_view).hash_code()};
}
vector<size_t> Option::OptionTypeTags = InitializeTags();

std::string Option::to_string() const {
    return format("[{}] {} {}", is_set ? "SET" : "UNSET", name, ::to_string(value));
}
#endif

#if 0
string to_string(const any &value) {
    if (!value.has_value()) {
        return string{};
    } else if (value.type().hash_code() == typeid(char).hash_code()) {
        auto converted_value = any_cast<char>(value);
        return to_string(converted_value);
    } else if (value.type().hash_code() == typeid(int8_t).hash_code()) {
        auto converted_value = any_cast<int8_t>(value);
        return to_string(converted_value);
    } else if (value.type().hash_code() == typeid(uint8_t).hash_code()) {
        auto converted_value = any_cast<uint8_t>(value);
        return to_string(converted_value);
    } else if (value.type().hash_code() == typeid(int16_t).hash_code()) {
        auto converted_value = any_cast<int16_t>(value);
        return to_string(converted_value);
    } else if (value.type().hash_code() == typeid(uint16_t).hash_code().hash_code()) {
        auto converted_value = any_cast<uint16_t>(value);
        return to_string(converted_value);
    } else if (value.type().hash_code() == typeid(int32_t).hash_code()) {
        auto converted_value = any_cast<int32_t>(value);
        return to_string(converted_value);
    } else if (value.type().hash_code() == typeid(uint32_t).hash_code()) {
        auto converted_value = any_cast<uint32_t>(value);
        return to_string(converted_value);
    } else if (value.type().hash_code() == typeid(int64_t).hash_code()) {
        auto converted_value = any_cast<int64_t>(value);
        return to_string(converted_value);
    } else if (value.type().hash_code() == typeid(uint64_t).hash_code()) {
        auto converted_value = any_cast<uint64_t>(value);
        return to_string(converted_value);
    } else if (value.type().hash_code() == typeid(float).hash_code()) {
        auto converted_value = any_cast<float>(value);
        return to_string(converted_value);
    } else if (value.type().hash_code() == typeid(double).hash_code()) {
        auto converted_value = any_cast<double>(value);
        return to_string(converted_value);
    } else if (value.type().hash_code() == typeid(string).hash_code()) {
        return any_cast<string>(value);
    } else if (value.type().hash_code() == typeid(string_view).hash_code()) {
        return string{any_cast<string_view>(value)};
    } else {
        const auto error_val = format("unknown std::any type {}", value.type().name());
        BOOST_LOG_TRIVIAL(error) << error_val;
        throw std::runtime_error{error_val};
    }
    return string{};
}
#endif

const AppOptions *AppOptions::Initialize(const AppOptionsCreator &creator) {
    static auto app_options = AppOptions{};
    app_options.AddOptions(creator);
    return &app_options;
}

void AppOptions::AddOptions(const AppOptionsCreator &creator) {
    options_ = creator.GetOptions();
}

/*
std::any AppOptions::GetOption(const std::string &name) const {
    auto i = find_if(begin(options_), end(options_), [&name](const auto &option) {
        return boost::iequals(name, option.name);
    });
    return i != end(options_) ? i->value : std::any{};
}

bool AppOptions::IsHelpNeeded() const { return false; }
*/
