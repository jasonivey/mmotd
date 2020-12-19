// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "app/include/cli_app_options_creator.h"
#include "app/include/logging.h"
#include "app/include/main.h"
#include "view/include/computer_information_provider.h"
#include "view/include/computer_information_provider_factory.h"

#include <cstdlib>
#include <iostream>
#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/range/iterator.hpp>
#include <fmt/color.h>
#include <fmt/format.h>

using namespace fmt;
using namespace std;

static const tuple<bool, const AppOptions *> LoadAppOptions(const int argc, char **argv) {
    const auto *app_options_creator = CliAppOptionsCreator::ParseCommandLine(argc, argv);
    if (app_options_creator->IsAppFinished()) {
        return make_tuple(app_options_creator->IsErrorExit(), nullptr);
    }
    auto *app_options = AppOptions::Initialize(*app_options_creator);
    return make_tuple(false, app_options);
}

static string GetInformation(const string &provider_name) {
    auto providers = mmotd::GetComputerInformationProviders();
    auto i = find_if(begin(providers), end(providers), [&provider_name](const auto &provider) {
        return provider->GetInformationName() == provider_name;
    });
    if (i == end(providers)) {
        return string{};
    }
    auto info = (*i)->GetComputerInformation();
    return info ? *info : string{};
}

static string GetSubHeader(const string &header) {
    auto final_str = format("  {}\n", header);
    auto i = header.find(":");
    if (i != string::npos) {
        final_str = format(fg(color::cyan) | emphasis::bold, "  {}", header.substr(0, i + 1));
        final_str += format(fg(color::white), "{}\n", header.substr(i + 1));
    }
    return final_str;
}

static string GetLastLogin(const string &login_info) {
    vector<string> login_parts;
    boost::split(login_parts, login_info, boost::is_any_of(","), boost::token_compress_on);
    auto final_str = format(fg(color::cyan) | emphasis::bold, "  {:<15}", "Last Login:");
    auto first = true;
    for (const auto &login_part : login_parts) {
        if (first) {
            final_str += format(" {}\n", login_part);
            first = false;
            continue;
        }
        auto i = login_part.find(":");
        if (i == string::npos) {
            final_str += format("{:18}{}\n", " ", login_part);
            continue;
        }
        final_str += format(fg(color::lime_green) | emphasis::bold, "{:17}{}", " ", login_part.substr(0, i + 1));
        final_str += format(fg(color::white), "{}\n", login_part.substr(i + 1));
    }
    return final_str;
}

static string GetRandomQuote(const string &quote) {
    if (!quote.empty()) {
        return format(fg(color::cyan) | emphasis::bold, "\n" + quote);
    }
    return string{};
}

static vector<tuple<string, string>> GetNetworkInterfaces(const string &network_interfaces) {
    PLOG_INFO << format("parsing network interfaces: {}", network_interfaces);
    auto result = vector<tuple<string, string>>{};
    auto network_regex = regex("([^:]+):\\s+([^,]+),\\s+(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})(,\\s+)?",
                               regex_constants::ECMAScript);
    auto regex_begin = sregex_iterator(begin(network_interfaces), end(network_interfaces), network_regex);
    auto regex_end = sregex_iterator{};
    for (auto i = regex_begin; i != regex_end; ++i) {
        auto match = *i;
        PLOG_INFO << format("found match with {} results", match.size());
        if (match.size() >= 3) {
            const auto &name = match[1].str();
            const auto &mac_addr = match[2].str();
            const auto &ip_addr = match[3].str();
            result.push_back(make_tuple(format("Mac {}", name), mac_addr));
            result.push_back(make_tuple(format("IP {}", name), ip_addr));
        }
    }
    return result;
}

static string FormatTwoColumns(const string &left_name,
                               const string &left_value,
                               const string &right_name,
                               const string &right_value) {
    static constexpr const size_t LEFT_NAME_COLUMN_WIDTH = 16;
    static constexpr const size_t LEFT_VALUE_COLUMN_WIDTH = 21;
    static constexpr const size_t RIGHT_NAME_COLUMN_WIDTH = 26;

    auto final_str = format(fg(color::cyan) | emphasis::bold,
                            "  {:<{}}",
                            left_name.empty() ? string{} : (left_name + ":"),
                            LEFT_NAME_COLUMN_WIDTH);
    final_str += format(fg(color::white), "{:<{}}", left_value, LEFT_VALUE_COLUMN_WIDTH);
    final_str += format(fg(color::cyan) | emphasis::bold,
                        "  {:<{}}",
                        right_name.empty() ? string{} : (right_name + ":"),
                        RIGHT_NAME_COLUMN_WIDTH);
    final_str += format(fg(color::white), "{}\n", right_value);
    return final_str;
}

static void PrintMmotd(const AppOptions & /*app_options*/) {
    print(fg(color::lime_green) | emphasis::bold, "{}\n\n", GetInformation("greeting"));

    print(fg(color::yellow), "  {}\n\n", GetInformation("header"));
    print(GetSubHeader(GetInformation("sub header")));
    print(GetLastLogin(GetInformation("last login")));

    print(fg(color::cyan) | emphasis::bold, "  {:<15}", "Boot Time:");
    print(fg(color::white), " {}\n\n", GetInformation("boot time"));

    print(FormatTwoColumns("Computer Name", GetInformation("computer name"), "Host Name", GetInformation("host name")));
    print(FormatTwoColumns("Public IP", GetInformation("public ip"), "Mail", "0 Unread Mail Items (fix)"));
    print(
        FormatTwoColumns("System Load", GetInformation("system load"), "Processes", GetInformation("processes count")));
    print(
        FormatTwoColumns("Usage of /", GetInformation("disk usage"), "Users Logged In", GetInformation("users count")));
    auto network_interfaces = GetNetworkInterfaces(GetInformation("active network interfaces"));
    auto memory_usage = GetInformation("memory usage");
    auto swap_usage = GetInformation("swap usage");
    for (const auto &network_interface : network_interfaces) {
        const auto &[name, addr] = network_interface;
        if (!memory_usage.empty()) {
            print(FormatTwoColumns("Memory Usage", memory_usage, name, addr));
            memory_usage.clear();
        } else if (!swap_usage.empty()) {
            print(FormatTwoColumns("Swap Usage", swap_usage, name, addr));
            swap_usage.clear();
        } else {
            print(FormatTwoColumns(string{}, string{}, name, addr));
        }
    }
    if (network_interfaces.empty()) {
        if (!memory_usage.empty()) {
            print(FormatTwoColumns("Memory Usage", memory_usage, string{}, string{}));
        }
        if (!swap_usage.empty()) {
            print(FormatTwoColumns("Swap Usage", swap_usage, string{}, string{}));
        }
    }
    print(GetRandomQuote(GetInformation("random quote")) + "\n");
}

int main(int argc, char *argv[]) {
    Logging::DefaultInitializeLogging();

    auto [error_encountered, app_options] = LoadAppOptions(argc, argv);
    if (app_options == nullptr) {
        return error_encountered ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    auto retval = EXIT_SUCCESS;
    try {
        if (app_options->GetOptions().IsVerboseSet()) {
            Logging::UpdateSeverityFilter(app_options->GetOptions().GetVerbosityLevel());
        }

        PrintMmotd(*app_options);
    } catch (boost::exception &ex) {
        auto diag = boost::diagnostic_information(ex);
        auto error_str = format("caught boost::exception in main: {}", diag);
        PLOG_FATAL << error_str;
        std::cerr << error_str << std::endl;
        retval = EXIT_FAILURE;
    } catch (const std::exception &ex) {
        auto error_str = format("caught std::exception in main: {}", ex.what());
        PLOG_FATAL << error_str;
        std::cerr << error_str << std::endl;
        retval = EXIT_FAILURE;
    } catch (...) {
        auto diag = boost::current_exception_diagnostic_information();
        auto error_str = format("caught unknown exception in main: {}", diag);
        PLOG_FATAL << error_str;
        std::cerr << error_str << std::endl;
        retval = EXIT_FAILURE;
    }

    return retval;
}
