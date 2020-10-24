// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "cli_app_options_creator.h"
#include "app_options.h"

#include <CLI/CLI.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <functional>
#include <fmt/format.h>
#include <any>
#include <algorithm>
#include <filesystem>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <fstream>

using namespace std;
using fmt::format;

template<class T>
string to_string(const vector<T> &values) {
    auto ostr_stream = ostringstream{};
    auto ostr_iterator = ostream_iterator<T>(ostr_stream, " ");
    copy(begin(values), end(values), ostr_iterator);
    return ostr_stream.str();
#if 0
    auto str = string{};
    for_each(begin(values), end(values), [&str](const auto &value) {
        if (!str.empty()) {
            str += " ";
        }
        str += value
    });
    return str;
#endif
}

CliAppOptionsCreator &CliAppOptionsCreator::GetInstance() {
    static auto cli_app_options_creator = CliAppOptionsCreator{};
    return cli_app_options_creator;
}

const CliAppOptionsCreator *CliAppOptionsCreator::ParseCommandLine(const int argc, char **argv) {
    auto &creator = GetInstance();
    creator.Parse(argc, argv);
    return &creator;
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
        BOOST_LOG_TRIVIAL(info) << msg;
        cout << msg << endl;
    } catch (const CLI::CallForVersion &version) {
        const string msg = format("version: {}", version.what());
        BOOST_LOG_TRIVIAL(info) << msg;
        cout << msg << endl;
    } catch (const CLI::ParseError &err) {
        // TODO: move this actual error all the way to main where we can actually use the error code
        if (err.get_exit_code() != 0) {
            BOOST_LOG_TRIVIAL(error) << format("error code {}: {}", err.get_exit_code(), err.what());
            cerr << format("ERROR ({}): {}\n", err.get_exit_code(), err.what());
        }
    }

    BOOST_LOG_TRIVIAL(debug) << "Options:\n" << to_string(options_) << endl;
    //ConfigTOML
    //std::string to_config(const App * /*app*/, bool default_also, bool write_description, std::string prefix) const override;

    //std::vector<ConfigItem> from_file(const std::string &name) {

    //std::vector<ConfigItem> from_config(std::istream &input) const override;

    //inline std::vector<ConfigItem> ConfigBase::from_config(std::istream &input) const {
}

void CliAppOptionsCreator::AddOptionDeclarations(CLI::App &app) {
    using std::placeholders::_1;

    //app.option_defaults()->configurable(true)->always_capture_default(true);
    app.option_defaults()->configurable(true)->multi_option_policy(CLI::MultiOptionPolicy::Throw);

    app.add_flag("-v,--verbose",
                 bind(&Options::SetVerbose, ref(options_), _1),
                 "increase output verbosity (can be specified multiple times, -vvv)")
                 ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll);

    // There is also an overload that takes a std::function to generate the version string dynamically
    app.set_version_flag("-V,--version", "0.0.1.git-rev");

    app.add_option("-l,--log-config",
                   bind(&Options::SetLogConfigPath, ref(options_), _1),
                   "logging config file used to specify log destinations, verbosity, rollover, etc.")
                   ->check(CLI::ExistingFile)
                   ->envname("MMOTD_LOG_CONFIG_PATH");

    app.set_config("-c,--config",
                   "",
                   "path to config file for specifying additional options")
                   ->check(CLI::ExistingFile)
                   ->envname("MMOTD_CONFIG_PATH");

    app.add_flag("--last-login,--no-last-login{false}",
                 bind(&Options::SetLastLogin, ref(options_), _1),
                 "display last login");

    app.add_flag("--computer-name,--no-computer-name{false}",
                 bind(&Options::SetComputerName, ref(options_), _1),
                 "display computer name");

    app.add_flag("--host-name,--no-host-name{false}",
                 bind(&Options::SetHostName, ref(options_), _1),
                 "display host name");

    app.add_flag("--public-ip,--no-public-ip{false}",
                 bind(&Options::SetPublicIp, ref(options_), _1),
                 "display public ip address");

    app.add_flag("--unread-mail,--no-unread-mail{false}",
                 bind(&Options::SetUnreadMail, ref(options_), _1),
                 "display unread mail");

    app.add_flag("--system-load,--no-system-load{false}",
                 bind(&Options::SetSystemLoad, ref(options_), _1),
                 "display system load");

    app.add_flag("--processor-count,--no-processor-count{false}",
                 bind(&Options::SetProcessorCount, ref(options_), _1),
                 "display processor count");

    app.add_flag("--disk-usage,--no-disk-usage{false}",
                 bind(&Options::SetDiskUsage, ref(options_), _1),
                 "display disk usage");

    app.add_flag("--users-count,--no-users-count{false}",
                 bind(&Options::SetUsersCount, ref(options_), _1),
                 "display users logged in count");

    app.add_flag("--memory-usage,--no-memory-usage{false}",
                 bind(&Options::SetMemoryUsage, ref(options_), _1),
                 "display memory usage");

    app.add_flag("--swap-usage,--no-swap-usage{false}",
                 bind(&Options::SetSwapUsage, ref(options_), _1),
                 "display swap usage");

    app.add_flag("--active-network-interfaces,--no-active-network-interfaces{false}",
                 bind(&Options::SetActiveNetworkInterfaces, ref(options_), _1),
                 "display active network interfaces (ip and mac address)");

    app.add_flag("--greeting,--no-greeting{false}",
                 bind(&Options::SetGreeting, ref(options_), _1),
                 "display greeting using user name, OS name, release and kernel");

    app.add_flag("--header,--no-header{false}",
                 bind(&Options::SetHeader, ref(options_), _1),
                 "display the system information header with date and time");

    app.add_flag("--sub-header,--no-sub-header{false}",
                 bind(&Options::SetSubHeader, ref(options_), _1),
                 "display the system information sub-header with location, weather, sunrise and sunset");

    app.add_flag("--random-programmer-quote,--no-random-programmer-quote{false}",
                 bind(&Options::SetQuote, ref(options_), _1),
                 "display a random programmer quote");
}
