// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/source_location_common.h"

#include <algorithm>
#include <string>
#include <string_view>

#include <fmt/format.h>

using fmt::format;
using namespace std;

namespace {

static constexpr std::string_view UNKNOWN_FILE = "<file>";
static constexpr std::string_view UNKNOWN_FUNCTION = "<...>";

size_t GetAnonymousNamespaceIndexEnd(string function) noexcept {
    static constexpr auto anonymous_ns1 = "(anonymous namespace)::"sv;
    static constexpr auto anonymous_ns2 = "{anonymous}::"sv;
    if (auto index1 = function.find(anonymous_ns1); index1 != string::npos) {
        return index1 + size(anonymous_ns1);
    } else if (auto index2 = function.find(anonymous_ns2); index2 != string::npos) {
        return index2 + size(anonymous_ns2);
    } else {
        return string::npos;
    }
}

pair<string::const_iterator, string::const_iterator> FindParens(string::const_iterator str_begin,
                                                                string::const_iterator str_end) noexcept {
    static constexpr auto OPEN_PAREN = "("sv;
    static constexpr auto CLOSE_PAREN = ")"sv;
    auto open_paren = find_first_of(str_begin, str_end, begin(OPEN_PAREN), end(OPEN_PAREN));
    if (open_paren == str_end) {
        return {str_end, str_end};
    }
    auto close_paren = find_first_of(open_paren, str_end, begin(CLOSE_PAREN), end(CLOSE_PAREN));
    if (close_paren == str_end) {
        return {str_end, str_end};
    }
    return {open_paren, close_paren};
}

string StripAroundParens(string function) noexcept {
    auto [open_paren, close_paren] = FindParens(begin(function), end(function));
    if (open_paren == end(function) || close_paren == end(function)) {
        // a valid result includes both the open and closes parens `(` and `)`
        //  (i.e. `void foo(int, float)`)
        //                  ^        ^
        return function;
    }
    return fmt::format(FMT_STRING("{}(...){}"),
                       string(cbegin(function), open_paren),
                       string(close_paren + 1, cend(function)));
}

} // namespace

namespace mmotd::source_location {

string TrimFileName(const char *file) noexcept {
    auto file_str = file == nullptr ? string{UNKNOWN_FILE} : string(file);
    if (auto index = file_str.find_last_of('/'); index != string::npos) {
        file_str = file_str.substr(index + 1);
    }
    return file_str;
}

// bool (anonymous namespace)::IsInterfaceActive(...)
// bool (anonymous namespace)::IsInterfaceActive(...)
// bool (anonymous namespace)::IsInterfaceActive(...)
// optional<fmt::text_style> (anonymous namespace)::GetTerminalPlainColor(...)
// optional<fmt::text_style> (anonymous namespace)::GetTerminalPlainColor(...)
// static optional<mmotd::information::Information> mmotd::results::TemplateString::FindInformation(...)
// static optional<std::__1::string> mmotd::results::TemplateString::GetInformationValue(...)
// static optional<mmotd::information::Information> mmotd::results::TemplateString::FindInformation(...)
// static optional<std::__1::string> mmotd::results::TemplateString::GetInformationValue(...)
// 'std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >' => 'string'
// boost::iterator_range<__gnu_cxx::__normal_iterator<char const*, std::__cxx11::basic_string<char,
// std::char_traits<char>, std::allocator<char> > > > boost::algorithm::detail::first_finderF<char const*,
// boost::algorithm::is_iequal>::operator()<__gnu_cxx::__normal_iterator<char const*, std::__cxx11::basic_string<char,
// std::char_traits<char>, std::allocator<char> > > >(__gnu_cxx::__normal_iterator<char const*,
// std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, __gnu_cxx::__normal_iterator<char
// const*, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >) const (finder.hpp:76)

string
TrimFunction(const char *function, FunctionArgStrategy arg_strategy, FunctionReturnStrategy return_strategy) noexcept {
    auto function_str = function == nullptr ? string{UNKNOWN_FUNCTION} : string(function);
    if (arg_strategy == FunctionArgStrategy::remove) {
        auto index = function_str.find('(');
        function_str = function_str.substr(0, index);
    } else if (arg_strategy == FunctionArgStrategy::replace) {
        function_str = StripFunctionArgs(function_str);
    }
    if (return_strategy == FunctionReturnStrategy::remove) {
        function_str = StripNamespaces(StripFunctionReturn(StripAnonymousNamespace(function_str)));
    }
    return function_str;
}

string StripFunctionReturn(string function) noexcept {
    if (auto index = function.rfind(' '); index != string::npos) {
        return function.substr(index + 1);
    } else {
        return function;
    }
}

string StripAnonymousNamespace(string function) noexcept {
    if (auto index = GetAnonymousNamespaceIndexEnd(function); index != string::npos) {
        return function.substr(index);
    } else {
        return function;
    }
}

string StripNamespaces(string function) noexcept {
    if (auto index = function.rfind("::"); index != string::npos) {
        if (auto sub_index = function.rfind("::", index - 1); sub_index != string::npos) {
            return function.substr(sub_index + 2);
        }
    }
    return function;
}

string StripFunctionArgs(string function) noexcept {
    return StripAroundParens(function);
}

} // namespace mmotd::source_location
