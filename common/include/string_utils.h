// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <string>
#include <string_view>

namespace mmotd::string_utils {

std::string RemoveAsciiEscapeCodes(std::string input);
std::string RemoveAsciiEscapeCodes(std::string_view input);

std::size_t GetAsciiEscapeCodesSize(std::string input);
std::size_t GetAsciiEscapeCodesSize(std::string_view input);

std::string RemoveMultibyteCharacters(std::string input);

inline std::string RemoveMultibyteAndEmbeddedColors(std::string input) {
    return RemoveMultibyteCharacters(RemoveAsciiEscapeCodes(input));
}

} // namespace mmotd::string_utils
