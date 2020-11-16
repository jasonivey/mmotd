#include "app/include/logging.h"

#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>

#include <cassert>
#include <cstddef>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

using fmt::format;
using namespace std;

template<typename... Args>
inline void unused(Args &&...) {
}

void Logging::DefaultInitializeLogging() {
    static auto file_appender = plog::RollingFileAppender<plog::TxtFormatter>("mmotd.log", 5 * 1048576, 3);
    static auto console_appender = plog::ColorConsoleAppender<plog::TxtFormatter>{};
    plog::init(plog::verbose, &file_appender);
    plog::init<CONSOLE_LOG>(plog::warning, &console_appender);
}

void Logging::InitializeLogging(const string & /*ini_file*/) {
    DefaultInitializeLogging();
}

void Logging::UpdateSeverityFilter(int /*verbosity*/) {
}
