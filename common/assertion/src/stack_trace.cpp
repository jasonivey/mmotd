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
#if defined(NDEBUG)
    return CLANG_RELEASE_LEVELS_TO_SKIP;
#else
    return CLANG_DEBUG_LEVELS_TO_SKIP;
#endif
#elif defined(__GNUG__)
#if defined(NDEBUG)
    return GCC_RELEASE_LEVELS_TO_SKIP;
#else
    return GCC_DEBUG_LEVELS_TO_SKIP;
#endif
#else
#if defined(NDEBUG)
    return GENERIC_RELEASE_LEVELS_TO_SKIP;
#else
    return GENERIC_DEBUG_LEVELS_TO_SKIP;
#endif
#endif
}
#if 0
string FormatSnippet(const char *indent, const backward::ResolvedTrace::SourceLoc &source_loc, unsigned context_size) {
    auto snippets = backward::SnippetFactory{};
    auto lines = snippets.get_snippet(source_loc.filename, source_loc.line, context_size);
    auto output = string{};
    for (auto i = begin(lines); i != end(lines); ++i) {
        if (i->first == source_loc.line) {
            output += format(FMT_STRING("{}>"), indent);
        } else {
            output += format(FMT_STRING("{} "), indent);
        }
        auto [line_number, line_content] = *i;
        output += format(FMT_STRING("{:4}: {}\n"), line_number, line_content);
    }
    return output;
}

string FormatSourceLoc(const char *indent, const backward::ResolvedTrace::SourceLoc &source_loc, void *addr = nullptr) {
    auto output = format(FMT_STRING("{}Source \"{}\", line {}, in {}"),
                         indent,
                         source_loc.filename,
                         source_loc.line,
                         source_loc.function);
    if (addr != nullptr) {
        output += format(FMT_STRING("[{}]"), fmt::ptr(addr));
    }
    return output + "\n";
}

string FormatTrace(const backward::ResolvedTrace &trace, int index, bool include_snippet) {
    static constexpr unsigned inliner_context_size = 5;
    static constexpr unsigned trace_context_size = 7;
    auto output = format(FMT_STRING("#{:2}"), index);
    bool already_indented = true;
    if (empty(trace.source.filename)) {
        output += format(FMT_STRING("   Object \"{}\", at {}, in {}\n"),
                         trace.object_filename,
                         trace.addr,
                         trace.object_function);
        already_indented = false;
    }
    for (size_t i = trace.inliners.size(); i > 0; --i) {
        if (!already_indented) {
            output += "   ";
        }
        const auto &source_loc = trace.inliners[i - 1];
        output += FormatSourceLoc(" | ", source_loc);
        if (include_snippet) {
            output += FormatSnippet("    | ", source_loc, inliner_context_size);
        }
        already_indented = false;
    }
    if (!empty(trace.source.filename)) {
        if (!already_indented) {
            output += "   ";
        }
        output += FormatSourceLoc("   ", trace.source, trace.addr);
        if (include_snippet) {
            output += FormatSnippet("      ", trace.source, trace_context_size);
        }
    }
    return output;
}

string GenerateStackTrace() {
    unused(GENERIC_DEBUG_LEVELS_TO_SKIP, CLANG_DEBUG_LEVELS_TO_SKIP, GCC_DEBUG_LEVELS_TO_SKIP);
    unused(GENERIC_RELEASE_LEVELS_TO_SKIP, CLANG_RELEASE_LEVELS_TO_SKIP, GCC_RELEASE_LEVELS_TO_SKIP);

    auto stack_trace = backward::StackTrace{};
    stack_trace.load_here();
    stack_trace.skip_n_firsts(GetLevelsToSkip());

    auto backtrace = string{};
    auto trace_resolver = backward::TraceResolver{};
    trace_resolver.load_stacktrace(stack_trace);
    for (auto i = size_t{0}; i != stack_trace.size(); ++i) {
        auto trace = trace_resolver.resolve(stack_trace[i]);
        backtrace += FormatTrace(trace, static_cast<int>(i), true);
    }
    return backtrace;
}
#else
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
#endif
} // namespace

namespace mmotd::assertion {

[[nodiscard]] string GetStackTrace() {
    return GenerateStackTrace();
}

} // namespace mmotd::assertion
