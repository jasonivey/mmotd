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

constexpr size_t CLANG_DEBUG_LEVELS_TO_SKIP = size_t{5};
constexpr size_t CLANG_RELEASE_LEVELS_TO_SKIP = size_t{1};
constexpr size_t GCC_DEBUG_LEVELS_TO_SKIP = size_t{5};
constexpr size_t GCC_RELEASE_LEVELS_TO_SKIP = size_t{1};
constexpr size_t GENERIC_DEBUG_LEVELS_TO_SKIP = size_t{0};
constexpr size_t GENERIC_RELEASE_LEVELS_TO_SKIP = size_t{0};

inline constexpr size_t GetLevelsToSkip() noexcept {
    unused(GENERIC_DEBUG_LEVELS_TO_SKIP,
           GENERIC_RELEASE_LEVELS_TO_SKIP,
           CLANG_DEBUG_LEVELS_TO_SKIP,
           CLANG_RELEASE_LEVELS_TO_SKIP,
           GCC_DEBUG_LEVELS_TO_SKIP,
           GCC_RELEASE_LEVELS_TO_SKIP);
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

string GenerateStackTrace() {
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

} // namespace

namespace mmotd::assertion {

[[nodiscard]] string GetStackTrace() {
    return GenerateStackTrace();
}

} // namespace mmotd::assertion
