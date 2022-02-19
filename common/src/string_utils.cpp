// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/algorithm.h"
#include "common/include/logging.h"
#include "common/include/string_utils.h"

#include <algorithm>
#include <cerrno>
#include <cwchar>
#include <iostream>
#include <iterator>
#include <regex>
#include <string>
#include <vector>

#include <utf8.h>

using namespace std;
using namespace std::string_literals;

namespace {

template<typename U, typename T>
U CopyTo(T input) {
    using src_type = typename U::value_type;
    using dst_type = typename T::value_type;
    auto output = string{};
    mmotd::algorithms::transform_if(
        begin(input),
        end(input),
        back_inserter(output),
        [](src_type input_char) {
            return input_char > src_type{0} && input_char <= static_cast<src_type>(numeric_limits<dst_type>::max());
        },
        [](src_type input_char) { return static_cast<dst_type>(input_char); });
    return output;
}

} // namespace

namespace mmotd::string_utils {

string RemoveAsciiEscapeCodes(string input) {
    // all ascii escape codes start with 0x1b, "[", codes within, ending with "m"
    const auto color_code_regex = regex(R"(\x1b\[[^m]+m)", regex::ECMAScript);
    return regex_replace(input, color_code_regex, "");
}

size_t GetAsciiEscapeCodesSize(string input) {
    return size(input) - size(RemoveAsciiEscapeCodes(input));
}

string RemoveMultibyteCharacters(string input) {
#if 1
    if (!utf8::is_valid(input)) {
        cerr << "invalid utf8: " << input << endl;
        input = utf8::replace_invalid(input);
    }
    u32string wide_input = utf8::utf8to32(input);
    auto output = string{};
    for (auto c : wide_input) {
        if (c < 0x80) {
            output += static_cast<char>(c);
        }
    }
    return output;
#else
    auto state = mbstate_t();
    const auto *input_ptr = data(input);
    auto wide_str_len = mbsrtowcs(nullptr, &input_ptr, 0, &state) + 1;
    auto wide_str = vector<wchar_t>(wide_str_len, 0);
    if (mbsrtowcs(data(wide_str), &input_ptr, size(wide_str), &state) == static_cast<size_t>(-1)) {
        return string{};
    }
    return CopyTo<string>(wide_str);
#endif
}

} // namespace mmotd::string_utils
