// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <cstdint>
#include <iosfwd>
#include <string>

namespace mmotd::source_location {

struct SourceLocation {
    static constexpr SourceLocation current(const char *file_name = __builtin_FILE(),
                                            const char *function_name = __builtin_FUNCTION(),
                                            int line = __builtin_LINE(),
                                            int column = 0) noexcept {
        return SourceLocation{file_name, function_name, line, column};
    }

    constexpr SourceLocation() noexcept = default;
    constexpr SourceLocation(const char *file_name, const char *function_name, long line, long column) noexcept :
        file_name_(file_name),
        function_name_(function_name),
        line_(static_cast<std::uint32_t>(line)),
        column_(static_cast<std::uint32_t>(column)) {}

    constexpr const char *file_name() const noexcept { return file_name_; }
    constexpr const char *function_name() const noexcept { return function_name_; }
    constexpr std::uint32_t line() const noexcept { return line_; }
    constexpr std::uint32_t column() const noexcept { return column_; }
    constexpr bool empty() const noexcept {
        return file_name_ == nullptr && function_name_ == nullptr && line_ == 0 && column_ == 0;
    }

private:
    const char *file_name_ = nullptr;
    const char *function_name_ = nullptr;
    std::uint32_t line_ = 0;
    std::uint32_t column_ = 0;
};

std::string file_name_to_string(const SourceLocation &location);
std::string function_name_to_string(const SourceLocation &location);

SourceLocation from_string(std::string input, std::string prefix = std::string{}, std::string suffix = std::string{});
std::string to_string(const SourceLocation &location);
std::ostream &operator<<(std::ostream &out, const SourceLocation &source_location);

} // namespace mmotd::source_location
