// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/stack_trace.h"
#include "common/include/algorithm.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include <backward.hpp>
#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

using namespace mmotd::algorithms;
using fmt::format;
using namespace std;

namespace {

static constexpr const size_t DEBUG_CLANG_LEVELS_TO_SKIP = size_t{6};
static constexpr const size_t DEBUG_GCC_LEVELS_TO_SKIP = size_t{5};
static constexpr const size_t DEBUG_LEVELS_TO_SKIP = size_t{0};
static constexpr const size_t RELEASE_CLANG_LEVELS_TO_SKIP = size_t{2};
static constexpr const size_t RELEASE_GCC_LEVELS_TO_SKIP = size_t{3};
static constexpr const size_t RELEASE_LEVELS_TO_SKIP = size_t{0};

inline constexpr size_t GetLevelsToSkip() noexcept {
#if defined(__GNUG__)
#    if defined(NDEBUG)
    return RELEASE_GCC_LEVELS_TO_SKIP;
#    else
    return DEBUG_GCC_LEVELS_TO_SKIP;
#    endif
#elif defined(__clang__)
#    if defined(NDEBUG)
    return RELEASE_CLANG_LEVELS_TO_SKIP;
#    else
    return DEBUG_CLANG_LEVELS_TO_SKIP;
#    endif
#else
#    if defined(NDEBUG)
    return RELEASE_LEVELS_TO_SKIP;
#    else
    return DEBUG_LEVELS_TO_SKIP;
#    endif
#endif
}

inline string generate_stack_trace() {
    unused(DEBUG_LEVELS_TO_SKIP, DEBUG_CLANG_LEVELS_TO_SKIP, DEBUG_GCC_LEVELS_TO_SKIP);
    unused(RELEASE_LEVELS_TO_SKIP, RELEASE_CLANG_LEVELS_TO_SKIP, RELEASE_GCC_LEVELS_TO_SKIP);

    auto stack_trace = backward::StackTrace{};
    stack_trace.load_here();
    stack_trace.skip_n_firsts(GetLevelsToSkip());
    auto printer = backward::Printer{};
    printer.color_mode = backward::ColorMode::never;
    auto ostrm = ostringstream{};
    printer.print(stack_trace, ostrm);
    return ostrm.str();
}

inline vector<string> split_stack_trace(string stack_trace_str) {
    auto stack_trace_lines = vector<string>{};
    boost::split(stack_trace_lines, stack_trace_str, boost::is_any_of("\n"), boost::token_compress_on);
    stack_trace_lines.erase(remove(begin(stack_trace_lines), end(stack_trace_lines), string{}), end(stack_trace_lines));
    return stack_trace_lines;
}

inline string reverse_stack_trace_impl(vector<string> stack_trace_lines) {
    if (empty(stack_trace_lines)) {
        return string{};
    }

    boost::replace_all(stack_trace_lines.front(), "most recent call last", "most recent call first");

    reverse(begin(stack_trace_lines) + 1, end(stack_trace_lines));

    auto index = 1;
    for_each(begin(stack_trace_lines) + 1, end(stack_trace_lines), [&index](auto &line) {
        auto index_str = format("#{:<5}", index++);
        copy(begin(index_str), end(index_str), begin(line));
    });

    return boost::join(stack_trace_lines, "\n");
}

inline string reverse_stack_trace(string stack_trace_str) {
    return reverse_stack_trace_impl(split_stack_trace(stack_trace_str));
}

} // namespace

namespace mmotd::assertion {

string GetStackTrace() {
    return reverse_stack_trace(generate_stack_trace());
}

} // namespace mmotd::assertion
