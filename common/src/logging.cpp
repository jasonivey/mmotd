// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/chrono_io.h"
#include "common/include/config_options.h"
#include "common/include/logging.h"
#include "common/include/source_location.h"
#include "common/include/version.h"

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <date/date.h>
#include <date/tz.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <sys/types.h>
#include <unistd.h>

namespace fs = std::filesystem;
using namespace mmotd::source_location;
using namespace std;

namespace {

inline uint32_t GetProcessId() {
    return static_cast<uint32_t>(getpid());
}

inline string GetBasename(string binary_name) {
    if (empty(binary_name)) {
        return string{"app"};
    }
    auto binary_path = fs::path{binary_name};
    return binary_path.has_stem() ? binary_path.stem().string() : string{"app"};
}

class FileLogger {
public:
    FileLogger() : file_stream_(), mutex_() {}
    ~FileLogger() { Close(); }
    FileLogger(const FileLogger &) = delete;
    FileLogger(const FileLogger &&) = delete;
    const FileLogger &operator=(const FileLogger &) = delete;
    const FileLogger &operator=(const FileLogger &&) = delete;

    void Open(const fs::path &file_path);
    void Close();

    void WriteLog(const fmt::memory_buffer &input, bool append_to_stderr = false);

    mmotd::logging::Severity GetSeverity() const noexcept;
    void SetSeverity(mmotd::logging::Severity severity) noexcept;

private:
    std::ofstream file_stream_;
    std::mutex mutex_;
    mmotd::logging::Severity severity_ = mmotd::logging::Severity::verbose;
};

inline mmotd::logging::Severity FileLogger::GetSeverity() const noexcept {
    return severity_;
}

inline void FileLogger::SetSeverity(mmotd::logging::Severity severity) noexcept {
    severity_ = severity;
}

void FileLogger::Open(const fs::path &file_path) {
    Close();
    auto lock = lock_guard<mutex>(mutex_);
    file_stream_.open(file_path);
    if (!file_stream_.is_open()) {
        auto sys_error = fmt::system_error(errno, FMT_STRING("cannot open file '{}'"), file_path.string());
        auto style = fmt::text_style(fmt::emphasis::bold) | fmt::fg(fmt::terminal_color::red);
        fmt::print(stderr, style, FMT_STRING("{}: {}"), sys_error.code(), sys_error.what());
    }
}

void FileLogger::Close() {
    auto lock = lock_guard<mutex>(mutex_);
    if (file_stream_.is_open()) {
        file_stream_.flush();
        file_stream_.close();
    }
    file_stream_ = std::ofstream{};
}

void FileLogger::WriteLog(const fmt::memory_buffer &input, bool append_to_stderr) {
    using namespace mmotd::logging;
    using mmotd::core::ConfigOptions;
    auto lock = lock_guard<mutex>(mutex_);
    if (file_stream_ && file_stream_.is_open()) {
        fmt::print(file_stream_, FMT_STRING("{}"), string_view(data(input), size(input)));
        if (ConfigOptions::Instance().GetValueAsBooleanOr("logging_flush", false)) {
            file_stream_.flush();
        }
    }
    if (append_to_stderr) {
        fmt::print(stderr, FMT_STRING("{}"), string_view(data(input), size(input)));
    }
}

// This is a short lived app -- intentionally create a leaked object so it is created on first demand
//  and lives beyond the scope of all global/function static variable destruction
FileLogger &GetFileLogger() {
    static auto file_logger = std::make_unique<FileLogger>();
    return *file_logger;
}

// This is storing the log file in "/tmp/mmotd-<proc-id>.log". Unique for each run of the app but will
//  be destroyed upon every boot. (Good for auto-cleanup of existing log files).
fs::path GetLoggingPath(const string &binary_name) {
    static const auto log_filename = format(FMT_STRING("{}-{:08x}.log"), GetBasename(binary_name), GetProcessId());
    static const auto default_log_path = fs::path("/tmp") / log_filename;
    return default_log_path;
}

inline bool HasHexPrefix(string num_value) {
    return size(num_value) >= 2 && num_value[0] == '0' && (num_value[1] == 'X' || num_value[1] == 'x');
}

string to_string(std::thread::id thread_id) {
    string thread_id_str = format(FMT_STRING("{}"), thread_id);
    if (HasHexPrefix(thread_id_str)) {
        thread_id_str = thread_id_str.substr(2);
    }
    if (size(thread_id_str) < 16) {
        thread_id_str = string(16 - size(thread_id_str), '0') + thread_id_str;
    }
    return thread_id_str;
}

string to_string(mmotd::logging::Severity severity) {
    using namespace mmotd::logging;
    switch (severity) {
        case Severity::fatal:
            return "FATAL";
            break;
        case Severity::error:
            return "ERROR";
            break;
        case Severity::warning:
            return "WARNING";
            break;
        case Severity::info:
            return "INFO";
            break;
        case Severity::debug:
            return "DEBUG";
            break;
        case Severity::verbose:
            return "VERBOSE";
            break;
        case Severity::none:
        default:
            return "NONE";
            break;
    }
}

fmt::text_style GetSeverityStyle(mmotd::logging::Severity severity) noexcept {
    using namespace mmotd::logging;
    auto style = fmt::text_style{};
    switch (severity) {
        case Severity::fatal:
            // fatal -> white on red background
            style = fmt::text_style(fmt::emphasis::bold) | fmt::fg(fmt::terminal_color::bright_white) |
                    fmt::bg(fmt::terminal_color::red);
            break;
        case Severity::error:
            // error -> red
            style = fmt::text_style(fmt::emphasis::bold) | fmt::fg(fmt::terminal_color::red);
            break;
        case Severity::warning:
            // warning -> yellow
            style = fmt::text_style(fmt::emphasis::bold) | fmt::fg(fmt::terminal_color::bright_yellow);
            break;
        case Severity::info:
            // info -> green
            style = fmt::text_style(fmt::emphasis::bold) | fmt::fg(fmt::terminal_color::bright_green);
            break;
        case Severity::debug:
            // debug -> orange
            style = fmt::text_style(fmt::emphasis::bold) | fmt::fg(fmt::color::orange);
            break;
        case Severity::verbose:
            // verbose -> cyan
            style = fmt::text_style(fmt::emphasis::bold) | fmt::fg(fmt::terminal_color::bright_cyan);
            break;
        case Severity::none:
        default:
            // none -> grey?
            style = fmt::text_style(fmt::emphasis::bold) | fmt::fg(fmt::color::gray);
            break;
    }
    return style;
}

void GetSourceLocationFormattedOutput(fmt::memory_buffer &out,
                                      mmotd::logging::Severity severity,
                                      const SourceLocation &source_location) {
    using namespace mmotd::logging;
    string thread_id_str = to_string(std::this_thread::get_id());
    auto now_time_point = date::make_zoned(mmotd::chrono::io::GetTimeZone(), std::chrono::system_clock::now());
    static constexpr size_t DATE_TIME_MILLISECOND_OFFSET = 23;
    static constexpr size_t SEVERITY_FIELD_WIDTH = 7;
    string now_str = date::format("%F %H:%M:%S", now_time_point).substr(0, DATE_TIME_MILLISECOND_OFFSET);
    fmt::format_to(std::back_inserter(out),
                   GetSeverityStyle(severity),
                   FMT_STRING("{} [{:^{}}] [{}] [{}] "),
                   now_str,
                   to_string(severity),
                   SEVERITY_FIELD_WIDTH,
                   thread_id_str,
                   to_string(source_location));
}

void GetFormattedOutput(fmt::memory_buffer &out,
                        mmotd::logging::Severity severity,
                        fmt::string_view format,
                        fmt::format_args args) {
    fmt::vformat_to(std::back_inserter(out), GetSeverityStyle(severity), format, args);
    if (out.size() != 0 && out[out.size() - 1] != '\n') {
        out.push_back('\n');
    }
}

void GetDirectFormattedOutput(fmt::memory_buffer &out, mmotd::logging::Severity severity, std::string_view msg) {
    fmt::format_to(std::back_inserter(out), GetSeverityStyle(severity), FMT_STRING("{}"), msg);
    if (out.size() != 0 && out[out.size() - 1] != '\n') {
        out.push_back('\n');
    }
}

void WriteLogHeader(const fs::path &binary_path) {
    auto version_str = mmotd::version::Version::Instance().to_string();
    auto app_name = binary_path.stem().string();
    auto date_time = mmotd::chrono::io::to_string(std::chrono::system_clock::now(), "%d-%h-%Y %I:%M%p %Z");
    auto header = fmt::format(FMT_STRING("{}: {} <{}>"), app_name, version_str, date_time);
    auto data = fmt::memory_buffer{};
    fmt::format_to(std::back_inserter(data),
                   FMT_STRING("┌{0:─^{2}}┐\n"
                              "│{1: ^{2}}│\n"
                              "└{0:─^{2}}┘\n\n"),
                   "",
                   header,
                   size(header) + 2);
    GetFileLogger().WriteLog(data);
}

void LogInternalImpl(const SourceLocation &source_location,
                     mmotd::logging::Severity severity,
                     const fmt::string_view &format,
                     fmt::format_args args) {
    auto data = fmt::memory_buffer{};
    GetSourceLocationFormattedOutput(data, severity, source_location);
    GetFormattedOutput(data, severity, format, args);
    GetFileLogger().WriteLog(data, severity == mmotd::logging::Severity::fatal);
}

void LogDirectInternalImpl(const SourceLocation &source_location,
                           mmotd::logging::Severity severity,
                           std::string_view &msg) {
    auto data = fmt::memory_buffer{};
    GetSourceLocationFormattedOutput(data, severity, source_location);
    GetDirectFormattedOutput(data, severity, msg);
    GetFileLogger().WriteLog(data, severity == mmotd::logging::Severity::fatal);
}

} // namespace

namespace mmotd::logging {

Severity GetSeverity() noexcept {
    return GetFileLogger().GetSeverity();
}

void SetSeverity(Severity severity, const SourceLocation &source_location) noexcept {
    auto output = fmt::memory_buffer{};
    auto previous_severity = GetSeverity();
    GetSourceLocationFormattedOutput(output, Severity::info, source_location);
    fmt::format_to(std::back_inserter(output),
                   GetSeverityStyle(Severity::info),
                   FMT_STRING("setting severity to '{}' when the previous value was '{}'\n"),
                   to_string(severity),
                   to_string(previous_severity));
    GetFileLogger().WriteLog(output);
    GetFileLogger().SetSeverity(severity);
}

void InitializeLogging(const string &binary_name) {
    auto logging_path = GetLoggingPath(binary_name);
    GetFileLogger().Open(logging_path);
    WriteLogHeader(binary_name);
}

void LogInternal(const SourceLocation &source_location,
                 Severity severity,
                 const fmt::string_view &format,
                 fmt::format_args args) {
    LogInternalImpl(source_location, severity, format, args);
}

void LogInternal(const mmotd::source_location::SourceLocation &source_location,
                 Severity severity,
                 const fmt::string_view &msg) {
    auto blank_args = fmt::format_args();
    LogInternalImpl(source_location, severity, msg, blank_args);
}

void LogDirectInternal(const mmotd::source_location::SourceLocation &source_location,
                       Severity severity,
                       std::string_view msg) {
    LogDirectInternalImpl(source_location, severity, msg);
}

} // namespace mmotd::logging
