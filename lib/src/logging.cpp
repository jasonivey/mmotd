#include "logging.h"

#include <cstddef>
#include <string>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/settings.hpp>
#include <boost/log/utility/setup/from_settings.hpp>
#include <boost/log/utility/setup/from_stream.hpp>
#include <boost/log/utility/setup/filter_parser.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
using namespace std;

void Logging::InitializeLogging(const string &ini_file) {

    if (boost::filesystem::exists(ini_file)) {
        cout << "loading logging settings from " << ini_file << "\n";
        auto ini_stream = ifstream(ini_file);
        logging::init_from_stream(ini_stream);
    } else {
        cout << "loading logging settings from hard-coded definitions\n";

        logging::settings setts;

        setts["Core"]["Filter"] = "%Severity% >= debug";
        setts["Core"]["DisableLogging"] = false;

        //setts["Sinks.Console"]["Destination"] = "Console";
        //setts["Sinks.Console"]["Filter"] = "%Severity% >= critical";
        //setts["Sinks.Console"]["Format"] = "%TimeStamp% [%Severity%] %Message%";
        //setts["Sinks.Console"]["AutoNewline"] = "InsertIfMissing";
        //setts["Sinks.Console"]["Asynchronous"] = false;
        //setts["Sinks.Console"]["AutoFlush"] = true;

        setts["Sinks.File"]["Destination"] = "TextFile";
        setts["Sinks.File"]["FileName"] = "mmotd.log";
        //setts["Sinks.File"]["FileName"] = "mmotd_%3N.log";
        setts["Sinks.File"]["Target"] = "/Users/jasoni/dev/";
        setts["Sinks.File"]["AutoFlush"] = true;
        setts["Sinks.File"]["AutoNewline"] = "InsertIfMissing";
        setts["Sinks.File"]["RotationSize"] = 10 * 1024 * 1024; // 10 MiB
        setts["Sinks.File"]["Format"] = "%TimeStamp% [%Severity%] %Message%";
        setts["Sinks.File"]["Append"] = true;

        logging::init_from_settings(setts);
    }
    logging::add_common_attributes();
}
