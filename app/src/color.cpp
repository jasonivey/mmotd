// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "app/include/color.h"

#include <string_view>

#include <fmt/color.h>

using namespace std;
using namespace fmt;
using namespace std::literals;

namespace mmotd::tty::color {

vector<tuple<text_style, string>> ParseFormatString(std::string s) {
    auto start = string::size_type{0};
    auto end = string::size_type{0};
    while (start != string::npos && end != string::npos) {
        start = s.find("##", end);
        if (start == string::npos) {
            continue;
        }
        end = s.find("##", start + "##"sv.size());
        if (end == string::npos) {
            continue;
        }
    }
    return vector<tuple<text_style, string>>{};
}

} // namespace mmotd::tty::color
