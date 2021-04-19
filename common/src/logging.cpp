// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/logging.h"

#include <cassert>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

#include <fmt/format.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

namespace mmotd::logging {

plog::Severity gFileAppenderVerbosity = plog::verbose;
plog::Severity gConsoleAppenderVerbosity = plog::warning;

void DefaultInitializeLogging(const string &filename) {
    static auto file_appender = plog::RollingFileAppender<plog::TxtFormatter>(filename.c_str(), 5 * 1048576, 3);
    static auto console_appender = plog::ColorConsoleAppender<plog::TxtFormatter>{};
    plog::init(gFileAppenderVerbosity, &file_appender);
    plog::init<CONSOLE_LOG>(gConsoleAppenderVerbosity, &console_appender);
}

// enum Severity
// {
//     none = 0,
//     fatal = 1,
//     error = 2,
//     warning = 3,
//     info = 4,
//     debug = 5,
//     verbose = 6
// };

static plog::Severity convert_verbosity(size_t verbosity) {
    if (verbosity == 0) {
        return plog::none;
    }
    if (static_cast<plog::Severity>(verbosity + 3) > plog::verbose) {
        return plog::verbose;
    } else {
        return static_cast<plog::Severity>(verbosity + 3);
    }
}

// verbosity is a 0-based value where 0=none, 1=info, 2=debug, 3=verbose
//  conversion from this range to the `plog::Severity` is a simple off by 3
//  calculation where the ceil is 6=verbose.
void UpdateSeverityFilter(size_t verbosity) {
    plog::Severity new_severity = convert_verbosity(verbosity);
    auto new_file_severity = std::max(new_severity, gFileAppenderVerbosity);
    auto new_console_severity = std::max(new_severity, gConsoleAppenderVerbosity);
    if (gFileAppenderVerbosity != new_file_severity) {
        gFileAppenderVerbosity = new_file_severity;
        auto *file_log = plog::get();
        file_log->setMaxSeverity(gFileAppenderVerbosity);
    }
    if (gConsoleAppenderVerbosity != new_console_severity) {
        gConsoleAppenderVerbosity = new_console_severity;
        auto *console_log = plog::get<CONSOLE_LOG>();
        console_log->setMaxSeverity(gConsoleAppenderVerbosity);
    }
}

} // namespace mmotd::logging
