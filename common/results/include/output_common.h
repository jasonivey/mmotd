// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <regex>
#include <string>

namespace mmotd::results::common {

inline std::string RemoveAsciiEscapeCodes(std::string input) {
    // all ascii escape codes start with 0x1b, "[", codes within, ending with "m"
    const auto color_code_regex = std::regex(R"(\x1b\[[^m]+m)", std::regex::ECMAScript);
    return std::regex_replace(input, color_code_regex, "");
}

} // namespace mmotd::results::common
