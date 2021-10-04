// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/source_location.h"
#include "common/include/source_location_common.h"

#include <charconv>
#include <ostream>
#include <regex>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

using namespace std;

namespace {

template<typename T>
T to_integer(string input) {
    T number{};
    auto [ptr, ec] = std::from_chars(data(input), &input[size(input)], number);
    return ec == std::errc{} ? number : T{};
}

mmotd::source_location::SourceLocation
from_string_impl(string input, string prefix = string{}, string suffix = string{}) {
    static const char *const pattern_str = R"(([-_\w\d\.]+(?:cpp|cc|c|hpp|hh|h|inl|i)?(?=@))@([^#]+)#(\d+):(\d+))";
    // fix_todo_jasoni: this is a very bad idea... taking random input from client and creating regex pattern
    auto pattern = prefix + pattern_str + suffix;
    auto source_location_regex = std::regex{pattern, regex::ECMAScript};
    auto matches = smatch{};
    if (regex_search(input, matches, source_location_regex) && matches.size() >= 5) {
        auto file_name = matches[1].str();
        auto function_name = matches[2].str();
        auto line_number = to_integer<long>(matches[3].str());
        auto column_number = to_integer<long>(matches[4].str());
        return mmotd::source_location::SourceLocation(data(file_name), data(function_name), line_number, column_number);
    }
    return mmotd::source_location::SourceLocation();
}

} // namespace

namespace mmotd::source_location {

string file_name_to_string(const SourceLocation &location) {
    return TrimFileName(location.file_name());
}

string function_name_to_string(const SourceLocation &location) {
    return TrimFunction(location.function_name(), FunctionArgStrategy::replace, FunctionReturnStrategy::remove);
}

SourceLocation from_string(string input, string prefix, string suffix) {
    return from_string_impl(input, prefix, suffix);
}

string to_string(const SourceLocation &location) {
    auto result = file_name_to_string(location);
    if (!empty(result)) {
        result += '@';
    }
    result += function_name_to_string(location);
    if (location.line() != 0) {
        result += fmt::format(FMT_STRING("#{}:{}"), location.line(), location.column());
    }
    return result;
}

ostream &operator<<(ostream &out, const SourceLocation &source_location) {
    out << to_string(source_location);
    return out;
}

} // namespace mmotd::source_location
