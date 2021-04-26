// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/chrono_io.h"
#include "common/include/logging.h"
#include "common/include/source_location.h"
#include "common/include/version.h"

#include <charconv>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <mutex>
#include <string>
#include <thread>

#include <date/date.h>
#include <date/tz.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace fs = std::filesystem;
using namespace std;

namespace mmotd::logging::logging_detail {

mmotd::logging::Severity LOGGING_SEVERITY = mmotd::logging::Severity::verbose;

}

namespace {

static std::mutex LOGGER_IO_MUTEX;

class FileLogger {
public:
    FileLogger() = default;
    ~FileLogger() { Close(); }

    void Open(const fs::path &file_path);
    void Close();

    void WriteLog(mmotd::logging::Severity severity, const fmt::memory_buffer &input, bool only_file_output = false);

private:
    std::FILE *file_ = nullptr;
    std::mutex mutex_;
};

void FileLogger::Open(const fs::path &file_path) {
    Close();
    auto lock = lock_guard<mutex>(mutex_);
    file_ = std::fopen(file_path.c_str(), "w");
    if (file_ == nullptr) {
        auto data = fmt::memory_buffer{};
        format_system_error(data, errno, fmt::format(FMT_STRING("cannot open file '{}'"), file_path.string()));
        auto style = fmt::text_style(fmt::emphasis::bold) | fmt::fg(fmt::terminal_color::red);
        fmt::print(stderr, style, FMT_STRING("{}"), string_view(data.data(), data.size()));
    }
}

void FileLogger::Close() {
    auto lock = lock_guard<mutex>(mutex_);
    if (file_ != nullptr) {
        std::fclose(file_);
        file_ = nullptr;
    }
}

void FileLogger::WriteLog(mmotd::logging::Severity severity, const fmt::memory_buffer &input, bool only_file_output) {
    using namespace mmotd::logging;
    auto lock = lock_guard<mutex>(mutex_);
    if (file_ != nullptr) {
        fmt::print(file_, FMT_STRING("{}"), string_view(data(input), size(input)));
    }
    if (!only_file_output && severity <= Severity::fatal) {
        fmt::print(stderr, FMT_STRING("{}"), string_view(data(input), size(input)));
    }
}

// This is a short lived app -- intentionally create a leaked object so it is created on first demand
//  and lives beyond the scope of all global/function static variable destruction
FileLogger *g_hidden_file_logger = nullptr;
FileLogger *GetFileLogger() {
    if (g_hidden_file_logger == nullptr) {
        g_hidden_file_logger = new FileLogger();
    }
    return g_hidden_file_logger;
}

fs::path GetLoggingPath(const string &binary_name) {
    if (!empty(binary_name)) {
        auto ec = error_code{};
        auto binary_path = fs::absolute(fs::path(binary_name), ec);
        if (!ec) {
            return binary_path.replace_extension("log");
        }
    }
    return fs::current_path() / "app.log";
}

inline constexpr bool HasHexPrefix(string_view num_value) {
    return size(num_value) >= 2 && num_value[0] == '0' && (num_value[1] == 'X' || num_value[1] == 'x');
}

string to_string(std::thread::id thread_id) {
    string thread_id_str = format(FMT_STRING("{}"), thread_id);
    if (HasHexPrefix(string_view(data(thread_id_str), size(thread_id_str)))) {
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
                                      const mmotd::source_location::SourceLocation &source_location) {
    using namespace mmotd::logging;
    string thread_id_str = to_string(std::this_thread::get_id());
    auto now_time_point = date::make_zoned(date::locate_zone("America/Denver"), std::chrono::system_clock::now());
    string now_str = date::format("%F %H:%M:%S", now_time_point).substr(0, 23);
    fmt::format_to(std::back_inserter(out),
                   GetSeverityStyle(severity),
                   FMT_STRING("{} [{:^7}] [{}] [{}] "),
                   now_str,
                   to_string(severity),
                   thread_id_str,
                   to_string(source_location));
}

void GetFormattedOutput(fmt::memory_buffer &out,
                        mmotd::logging::Severity severity,
                        fmt::string_view format,
                        fmt::format_args args) {
    fmt::vformat_to(std::back_inserter(out), GetSeverityStyle(severity), format, args);
    if (out.size() != 0 && out.data()[out.size() - 1] != '\n') {
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
    GetFileLogger()->WriteLog(mmotd::logging::Severity::fatal, data, true);
}

} // namespace

namespace mmotd::logging {

void InitializeLogging(const string &binary_name) {
    auto logging_path = GetLoggingPath(binary_name);
    GetFileLogger()->Open(logging_path);
    WriteLogHeader(binary_name);
}

void LogInternal(const mmotd::source_location::SourceLocation &source_location,
                 Severity severity,
                 fmt::string_view format,
                 fmt::format_args args) {
    auto data = fmt::memory_buffer{};
    GetSourceLocationFormattedOutput(data, severity, source_location);
    GetFormattedOutput(data, severity, format, args);
    GetFileLogger()->WriteLog(severity, data);
}

} // namespace mmotd::logging
