// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/coloring.h"
#include "common/include/logging.h"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iterator>
#include <string>

#include <boost/algorithm/string.hpp>
#include <fmt/ostream.h>

using namespace std;
using fmt::format;

namespace mmotd::core::colors {

string ConvertStringToHexChars(string input) {
    auto output_str = string{};
    for (const auto input_char : input) {
        output_str += fmt::format(FMT_STRING("{:02x} "), static_cast<uint32_t>(input_char));
    }
    return string{"["} + output_str.substr(0, size(output_str) - 1) + string{"]"};
}

string RemoveNonAsciiCharsCopy(string input) {
    LOG_VERBOSE("remove emoji input: {}", ConvertStringToHexChars(input));
    LOG_VERBOSE("remove emoji input: {}", quoted(input));
    auto output = string{};
    copy_if(begin(input), end(input), back_inserter(output), [](char char_value) {
        return static_cast<uint32_t>(char_value) < 0x80;
    });
    if (size(input) > size(output)) {
        boost::replace_all(output, "  ", " ");
        boost::replace_all(output, " ,", ",");
    }
    LOG_VERBOSE("remove emoji output: {}", ConvertStringToHexChars(output));
    LOG_VERBOSE("remove emoji input: {}, output: {}", quoted(input), quoted(output));
    return output;
}

void RemoveNonAsciiChars(string &str) {
    LOG_VERBOSE("remove emoji input: {}", ConvertStringToHexChars(str));
    LOG_VERBOSE("remove emoji input: {}", quoted(str));
    auto i = remove_if(begin(str), end(str), [](char char_value) { return static_cast<uint32_t>(char_value) >= 0x80; });
    if (i != end(str)) {
        str.erase(i, end(str));
        boost::replace_all(str, "  ", " ");
        boost::replace_all(str, " ,", ",");
    }
    LOG_VERBOSE("remove emoji output: {}", ConvertStringToHexChars(str));
    LOG_VERBOSE("remove emoji output: {}", quoted(str));
}

} // namespace mmotd::core::colors
