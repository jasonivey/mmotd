// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <plog/Log.h>
#include <string>

namespace Logging {

const constexpr int CONSOLE_LOG = 1;

void DefaultInitializeLogging();
void InitializeLogging(const std::string &ini_file);
void UpdateSeverityFilter(int severity);

} // namespace Logging

#define MMOTD_LOG_VERBOSE(msg_)                             \
    do {                                                    \
        PLOG(plog::verbose) << msg_;                        \
        PLOG_(Logging::CONSOLE_LOG, plog::verbose) << msg_; \
    } while (false)
#define MMOTD_LOG_DEBUG(msg_)                             \
    do {                                                  \
        PLOG(plog::debug) << msg_;                        \
        PLOG_(Logging::CONSOLE_LOG, plog::debug) << msg_; \
    } while (false)
#define MMOTD_LOG_INFO(msg_)                             \
    do {                                                 \
        PLOG(plog::info) << msg_;                        \
        PLOG_(Logging::CONSOLE_LOG, plog::info) << msg_; \
    } while (false)
#define MMOTD_LOG_WARNING(msg_)                             \
    do {                                                    \
        PLOG(plog::warning) << msg_;                        \
        PLOG_(Logging::CONSOLE_LOG, plog::warning) << msg_; \
    } while (false)
#define MMOTD_LOG_ERROR(msg_)                             \
    do {                                                  \
        PLOG(plog::error) << msg_;                        \
        PLOG_(Logging::CONSOLE_LOG, plog::error) << msg_; \
    } while (false)
#define MMOTD_LOG_FATAL(msg_)                             \
    do {                                                  \
        PLOG(plog::fatal) << msg_;                        \
        PLOG_(Logging::CONSOLE_LOG, plog::fatal) << msg_; \
    } while (false)
#define MMOTD_LOG_NONE(msg_)                             \
    do {                                                 \
        PLOG(plog::none) << msg_;                        \
        PLOG_(Logging::CONSOLE_LOG, plog::none) << msg_; \
    } while (false)
