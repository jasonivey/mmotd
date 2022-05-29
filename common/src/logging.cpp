// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/logging.h"

#include "common/assertion/include/precondition.h"
#include "common/include/chrono_io.h"
#include "common/include/source_location.h"
#include "common/include/version.h"

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

#include <spdlog/async.h>
#include <spdlog/sinks/syslog_sink.h>
#include <spdlog/spdlog.h>
#include <toml/comments.hpp>

using namespace std;
using namespace std::string_literals;
using mmotd::logging::Severity;
using mmotd::source_location::SourceLocation;

namespace {

#if defined(NDEBUG)
static constexpr auto DEFAULT_LOGGING_LEVEL = spdlog::level::trace;
#else
static constexpr auto DEFAULT_LOGGING_LEVEL = spdlog::level::trace;
#endif

inline spdlog::source_loc to_source_loc(const SourceLocation &location) {
    return spdlog::source_loc{location.file_name(), static_cast<int>(location.line()), location.function_name()};
}

inline spdlog::level::level_enum to_level_enum(Severity severity) {
    return static_cast<spdlog::level::level_enum>(severity);
}

inline Severity to_severity(spdlog::level::level_enum lvl) {
    return static_cast<mmotd::logging::Severity>(lvl);
}

inline string GetBasename(string_view binary_name) {
    namespace fs = std::filesystem;
    static const auto GENERIC_APP_NAME = string{"app"};
    if (empty(binary_name)) {
        return GENERIC_APP_NAME;
    }
    auto binary_path = fs::path{binary_name};
    return binary_path.has_stem() ? binary_path.stem().string() : GENERIC_APP_NAME;
}

constexpr bool AssertLoggingLevelsAreEqual() {
    using spdlog::level::level_enum;
    static_assert(static_cast<int>(Severity::trace) == level_enum::trace, "Severity::trace != spdlog::trace");
    static_assert(static_cast<int>(Severity::debug) == level_enum::debug, "Severity::debug != spdlog::debug");
    static_assert(static_cast<int>(Severity::info) == level_enum::info, "Severity::info != spdlog::info");
    static_assert(static_cast<int>(Severity::warn) == level_enum::warn, "Severity::warn != spdlog::warn");
    static_assert(static_cast<int>(Severity::err) == level_enum::err, "Severity::err != spdlog::err");
    static_assert(static_cast<int>(Severity::critical) == level_enum::critical,
                  "Severity::critical != spdlog::critical");
    static_assert(static_cast<int>(Severity::off) == level_enum::off, "Severity::off != spdlog::off");
    return true;
}

class LoggingImpl {
public:
    explicit LoggingImpl(string_view binary_name);
    LoggingImpl(LoggingImpl const &other) = delete;
    LoggingImpl &operator=(LoggingImpl const &other) = delete;
    LoggingImpl(LoggingImpl &&other) = delete;
    LoggingImpl &operator=(LoggingImpl &&other) = delete;
    ~LoggingImpl();

    static LoggingImpl &Instance(string_view name = string_view{});

    bool WriteLogHeader(string_view app_name);

    mmotd::logging::Severity SetSeverity(mmotd::logging::Severity severity);

    void Log(const SourceLocation &location, Severity severity, const fmt::string_view &msg);
    void Log(const SourceLocation &location, Severity severity, const fmt::string_view &msg, fmt::format_args args);

private:
    string logger_name_;
    shared_ptr<spdlog::logger> logger_;
};

LoggingImpl::LoggingImpl(string_view binary_name) :
    logger_name_(GetBasename(binary_name)),
    logger_(spdlog::syslog_logger_mt<spdlog::async_factory>(logger_name_, "", LOG_PID, LOG_USER, true)) {
    spdlog::set_default_logger(logger_);
    spdlog::set_level(DEFAULT_LOGGING_LEVEL);
    spdlog::set_pattern("[%t] [%l] [%s:%!:%#] %v");
    AssertLoggingLevelsAreEqual();
}

LoggingImpl::~LoggingImpl() {
    if (logger_) {
        logger_->flush();
        logger_.reset();
    }
    spdlog::shutdown();
}

LoggingImpl &LoggingImpl::Instance(string_view name) {
    static auto instance = LoggingImpl(name);
    return instance;
}

void LoggingImpl::Log(const SourceLocation &location, Severity severity, const fmt::string_view &msg) {
    PRECONDITIONS(logger_, "logger must be initialized before logging");
    logger_->log(to_source_loc(location), to_level_enum(severity), msg);
}

void LoggingImpl::Log(const SourceLocation &location,
                      Severity severity,
                      const fmt::string_view &msg,
                      fmt::format_args args) {
    PRECONDITIONS(logger_, "logger must be initialized before logging");
    auto formatted_msg = fmt::vformat(msg, args);
    logger_->log(to_source_loc(location), to_level_enum(severity), formatted_msg);
    // fmt::print(stdout,
    //            FMT_STRING("[{}] [{}:{}:{}]: {}\n"),
    //            to_string(severity),
    //            location.file_name(),
    //            location.line(),
    //            location.function_name(),
    //            formatted_msg);
}

bool LoggingImpl::WriteLogHeader(string_view app_name) {
    auto version_str = mmotd::version::Version::Instance().to_string();
    // "Feb 12 1996 23:59:01"
    static constexpr string_view BUILD_DATE_TIME = __DATE__ " " __TIME__;
    static const auto build_date = mmotd::chrono::io::date_time_from_string(BUILD_DATE_TIME, "%b %d %Y %H:%M:%S");
    auto build_date_str = std::string{"UNKNOWN"};
    if (build_date) {
        build_date_str = mmotd::chrono::io::to_string(*build_date, "%d-%h-%Y %I:%M%p %Z");
    }
    mmotd::logging::LogCommon(mmotd::source_location::SourceLocation::current(),
                              mmotd::logging::Severity::info,
                              FMT_STRING("Starting {}: v{} built on <{}>"),
                              app_name,
                              version_str,
                              build_date_str);
    return true;
}

mmotd::logging::Severity LoggingImpl::SetSeverity(mmotd::logging::Severity new_severity) {
    auto prev_severity = to_severity(spdlog::get_level());
    spdlog::set_level(to_level_enum(new_severity));
    return prev_severity;
}

} // namespace

namespace mmotd::logging {

bool InitializeLogging(string_view binary_name) {
    auto &logging_impl = LoggingImpl::Instance(binary_name);
    return logging_impl.WriteLogHeader(binary_name);
}

mmotd::logging::Severity SetSeverity(mmotd::logging::Severity severity) {
    auto &logging_impl = LoggingImpl::Instance();
    return logging_impl.SetSeverity(severity);
}

std::string to_string(Severity severity) {
    switch (severity) {
        case Severity::trace:
            return "verbose";
        case Severity::debug:
            return "debug";
        case Severity::info:
            return "info";
        case Severity::warn:
            return "warning";
        case Severity::err:
            return "error";
        case Severity::critical:
            return "fatal";
        case Severity::off:
            return "off";
        default:
            return "unknown";
    }
}

namespace detail {

void LogInternal(const SourceLocation &source_location, Severity severity, const fmt::string_view &msg) {
    auto &logger = LoggingImpl::Instance();
    logger.Log(source_location, severity, msg);
}

void LogInternal(const SourceLocation &source_location,
                 Severity severity,
                 const fmt::string_view &msg,
                 fmt::format_args args) {
    auto &logger = LoggingImpl::Instance();
    logger.Log(source_location, severity, msg, args);
}

} // namespace detail

} // namespace mmotd::logging
