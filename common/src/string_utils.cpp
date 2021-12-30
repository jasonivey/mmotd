// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/algorithm.h"
#include "common/include/string_utils.h"

#include <algorithm>
#include <cerrno>
#include <cwchar>
#include <iterator>
#include <regex>
#include <string>
#include <vector>

using namespace std;
using namespace std::string_literals;

namespace {

template<typename T>
string CopyFrom(T input) {
    auto output = string{};
    mmotd::algorithms::transform_if(
        begin(input),
        end(input),
        back_inserter(output),
        [](wchar_t input_char) {
            return input_char > 0 && input_char <= static_cast<wchar_t>(numeric_limits<char>::max());
        },
        [](wchar_t input_char) { return static_cast<char>(input_char); });
    return output;
}

} // namespace

namespace mmotd::string_utils {

string RemoveAsciiEscapeCodes(string input) {
    // all ascii escape codes start with 0x1b, "[", codes within, ending with "m"
    const auto color_code_regex = regex(R"(\x1b\[[^m]+m)", regex::ECMAScript);
    return regex_replace(input, color_code_regex, "");
}

string RemoveMultibyteCharacters(string input) {
    auto state = mbstate_t();
    const auto *input_ptr = data(input);
    auto wide_str_len = mbsrtowcs(nullptr, &input_ptr, 0, &state) + 1;
    auto wide_str = vector<wchar_t>(wide_str_len, 0);

    if (mbsrtowcs(data(wide_str), &input_ptr, size(wide_str), &state) == static_cast<size_t>(-1)) {
        return string{};
    }
    return CopyFrom(wide_str);
}

} // namespace mmotd::string_utils
