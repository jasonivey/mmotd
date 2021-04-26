// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/source_location.h"

#include <string>

#include <boost/current_function.hpp>
#include <fmt/color.h>
#include <fmt/format.h>

namespace mmotd::logging {

enum class Severity { none = 0, fatal = 1, error = 2, warning = 3, info = 4, debug = 5, verbose = 6 };

namespace logging_detail {

extern mmotd::logging::Severity LOGGING_SEVERITY;

}

inline Severity GetSeverity() noexcept {
    return mmotd::logging::logging_detail::LOGGING_SEVERITY;
}

inline void UpdateSeverity(Severity severity) {
    mmotd::logging::logging_detail::LOGGING_SEVERITY = severity;
}

void InitializeLogging(const std::string &binary_name);

void LogInternal(const mmotd::source_location::SourceLocation &source_location,
                 Severity severity,
                 fmt::string_view format,
                 fmt::format_args args);

template<typename S, typename... Args>
void Log(const char *file, long line, const char *function, Severity severity, const S &format, Args &&...args) {
    LogInternal(mmotd::source_location::SourceLocation(file, line, function),
                severity,
                format,
                fmt::make_args_checked<Args...>(format, args...));
}

} // namespace mmotd::logging

#define LOG_FATAL(format, ...)                                                  \
    do {                                                                        \
        if (mmotd::logging::GetSeverity() >= mmotd::logging::Severity::fatal) { \
            mmotd::logging::Log(__FILE__,                                       \
                                __LINE__,                                       \
                                BOOST_CURRENT_FUNCTION,                         \
                                mmotd::logging::Severity::fatal,                \
                                FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__); \
        }                                                                       \
    } while (false)
#define LOG_ERROR(format, ...)                                                  \
    do {                                                                        \
        if (mmotd::logging::GetSeverity() >= mmotd::logging::Severity::error) { \
            mmotd::logging::Log(__FILE__,                                       \
                                __LINE__,                                       \
                                BOOST_CURRENT_FUNCTION,                         \
                                mmotd::logging::Severity::error,                \
                                FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__); \
        }                                                                       \
    } while (false)
#define LOG_WARNING(format, ...)                                                  \
    do {                                                                          \
        if (mmotd::logging::GetSeverity() >= mmotd::logging::Severity::warning) { \
            mmotd::logging::Log(__FILE__,                                         \
                                __LINE__,                                         \
                                BOOST_CURRENT_FUNCTION,                           \
                                mmotd::logging::Severity::warning,                \
                                FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__);   \
        }                                                                         \
    } while (false)
#define LOG_INFO(format, ...)                                                   \
    do {                                                                        \
        if (mmotd::logging::GetSeverity() >= mmotd::logging::Severity::info) {  \
            mmotd::logging::Log(__FILE__,                                       \
                                __LINE__,                                       \
                                BOOST_CURRENT_FUNCTION,                         \
                                mmotd::logging::Severity::info,                 \
                                FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__); \
        }                                                                       \
    } while (false)
#define LOG_DEBUG(format, ...)                                                  \
    do {                                                                        \
        if (mmotd::logging::GetSeverity() >= mmotd::logging::Severity::debug) { \
            mmotd::logging::Log(__FILE__,                                       \
                                __LINE__,                                       \
                                BOOST_CURRENT_FUNCTION,                         \
                                mmotd::logging::Severity::debug,                \
                                FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__); \
        }                                                                       \
    } while (false)
#define LOG_VERBOSE(format, ...)                                                  \
    do {                                                                          \
        if (mmotd::logging::GetSeverity() >= mmotd::logging::Severity::verbose) { \
            mmotd::logging::Log(__FILE__,                                         \
                                __LINE__,                                         \
                                BOOST_CURRENT_FUNCTION,                           \
                                mmotd::logging::Severity::verbose,                \
                                FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__);   \
        }                                                                         \
    } while (false)
