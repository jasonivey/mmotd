#include "options.h"

#include <optional>
#include <boost/exception/all.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <stdexcept>

using namespace std;
namespace po = boost::program_options;

static auto g_app_options = std::optional<AppOptions>{};

Option::Option(const string &name, const boost::any &value) :
    name(name),
    value(value)
{}

//AppOptions::AppOptions(bool) :
//    options_() {}

optional<AppOptions> AppOptions::Initialize(int argc, char **argv) {
    auto &app_options = AppOptions::GetAppOptions();
    try {
        if (!app_options.ParseCommandLine(argc, argv)) {
            cerr << "Parse command line failed\n";
            return optional<AppOptions>{};
        }
        g_app_options = make_optional(app_options);
        return optional<AppOptions>{app_options};
    } catch (po::invalid_command_line_syntax &ex) {
        cerr << "ERROR: invalid command line syntax. " << ex.what() << "\n";
    //} catch (boost::exception &ex) {
    //    cerr << "ERROR: exception raised. " << boost::diagnostic_information(ex) << "\n";
    } catch (const std::exception &ex) {
        cerr << "ERROR: exception raised. " << ex.what() << "\n";
    }
    return optional<AppOptions>{};
}

AppOptions &AppOptions::GetAppOptions() {
    static auto app_options = AppOptions{};
    return app_options;
}

bool AppOptions::ParseCommandLine(int argc, char **argv) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("verbose,v", po::value<int>()->default_value(0), "set the verbosity level")
        ("config", po::value<string>(), "set compression level")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << desc << "\n";
        return false;
    }

    if (vm.count("verbose")) {
        cout << "verbosity level was set to " << vm["verbose"].as<int>() << "\n";
    } else {
        cout << "verbosity level was not set\n";
    }
    return true;
}
