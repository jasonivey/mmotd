// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/stack_trace.h"
#include "common/include/algorithm.h"
#include "common/include/logging.h"
#include "common/include/source_location_common.h"

#include <algorithm>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <backward.hpp>
#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

using namespace mmotd::algorithms;
using fmt::format;
using mmotd::source_location::StripFunctionArgs;
using namespace std;

namespace {

static constexpr const size_t CLANG_DEBUG_LEVELS_TO_SKIP = size_t{6};
static constexpr const size_t CLANG_RELEASE_LEVELS_TO_SKIP = size_t{1};
static constexpr const size_t GCC_DEBUG_LEVELS_TO_SKIP = size_t{5};
static constexpr const size_t GCC_RELEASE_LEVELS_TO_SKIP = size_t{1};
static constexpr const size_t GENERIC_DEBUG_LEVELS_TO_SKIP = size_t{0};
static constexpr const size_t GENERIC_RELEASE_LEVELS_TO_SKIP = size_t{0};

inline constexpr size_t GetLevelsToSkip() noexcept {
#if defined(__clang__)
#    if defined(NDEBUG)
    return CLANG_RELEASE_LEVELS_TO_SKIP;
#    else
    return CLANG_DEBUG_LEVELS_TO_SKIP;
#    endif
#elif defined(__GNUG__)
#    if defined(NDEBUG)
    return GCC_RELEASE_LEVELS_TO_SKIP;
#    else
    return GCC_DEBUG_LEVELS_TO_SKIP;
#    endif
#else
#    if defined(NDEBUG)
    return GENERIC_RELEASE_LEVELS_TO_SKIP;
#    else
    return GENERIC_DEBUG_LEVELS_TO_SKIP;
#    endif
#endif
}

string GenerateStackTrace() {
    unused(GENERIC_DEBUG_LEVELS_TO_SKIP, CLANG_DEBUG_LEVELS_TO_SKIP, GCC_DEBUG_LEVELS_TO_SKIP);
    unused(GENERIC_RELEASE_LEVELS_TO_SKIP, CLANG_RELEASE_LEVELS_TO_SKIP, GCC_RELEASE_LEVELS_TO_SKIP);

    auto stack_trace = backward::StackTrace{};
    stack_trace.load_here();
    stack_trace.skip_n_firsts(GetLevelsToSkip());
    auto printer = backward::Printer{};
    printer.color_mode = backward::ColorMode::automatic;
    printer.snippet = false;
    printer.object = true;
    printer.address = true;
    auto ostrm = ostringstream{};
    printer.print(stack_trace, ostrm);
    return ostrm.str();
}

vector<string> SplitStackTrace(string stack_trace_str) {
    auto backtrace_details = vector<string>{};
    boost::split(backtrace_details, stack_trace_str, boost::is_any_of("\n"), boost::token_compress_on);
    backtrace_details.erase(remove(begin(backtrace_details), end(backtrace_details), string{}), end(backtrace_details));
    return backtrace_details;
}

bool IsFirstOfStackDetails(string detail) {
    const auto number_prefix = regex(R"(^#\d+\s+)", regex_constants::ECMAScript);
    return regex_search(detail, number_prefix, regex_constants::match_continuous);
}

void RemovePreMainStackDetails(vector<vector<string>> &details) {
    using namespace mmotd::algorithms;
    auto i = find_last_if(begin(details), end(details), [](const auto &sub_details) {
        auto j = find_if(begin(sub_details), end(sub_details), [](const auto &detail) {
            return !boost::ifind_first(detail, ", in main [").empty();
        });
        return j != end(sub_details);
    });
    if (i == end(details)) {
        LOG_VERBOSE("main was not found within the stack trace");
        return;
    }
    const auto end_index = static_cast<size_t>(distance(begin(details), i));
    auto index = size_t{0};
    auto j = remove_if(begin(details), end(details), [end_index, &index](const auto &) { return end_index > index++; });
    details.erase(j, end(details));
    details.shrink_to_fit();
}

string ReverseStackTraceImpl(vector<string> backtrace_details) {
    if (empty(backtrace_details)) {
        return string{};
    }
    auto header = boost::replace_all_copy(backtrace_details.front(), "most recent call last", "most recent call first");
    auto seperated_details = vector<vector<string>>{};
    for (auto i = begin(backtrace_details) + 1; i != end(backtrace_details); ++i) {
        const auto &detail = *i;
        if (IsFirstOfStackDetails(detail)) {
            seperated_details.push_back(vector<string>{detail});
        } else {
            MMOTD_CHECKS(!empty(seperated_details), "unable to seperate stack trace details w/out line with #{num}");
            seperated_details.back().push_back(detail);
        }
    }
    RemovePreMainStackDetails(seperated_details);
    // reverse the backtrace details and re-index the numeric prefixes to be increasing
    auto count = size_t{1}; // human readable lists start with #1 and increase from there
    auto new_backtrace = vector<string>{1, header};
    for (auto i = rbegin(seperated_details); i != rend(seperated_details); ++i) {
        auto details = *i;
        for (auto j = begin(details); j != end(details); ++j) {
            auto &sub_detail = *j;
            if (j == begin(details)) {
                auto index_str = format(FMT_STRING("#{:<5}"), count++);
                copy(begin(index_str), end(index_str), begin(sub_detail));
            }
            new_backtrace.push_back(StripFunctionArgs(sub_detail));
        }
    }
    return boost::join(new_backtrace, "\n");
}

string ReverseStackTrace(string stack_trace_str) {
    return ReverseStackTraceImpl(SplitStackTrace(stack_trace_str));
}

} // namespace

namespace mmotd::assertion {

string GetStackTrace() {
    return ReverseStackTrace(GenerateStackTrace());
    // return GenerateStackTrace();
}

} // namespace mmotd::assertion
