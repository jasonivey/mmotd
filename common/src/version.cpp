#include "common/include/version.h"
#include "common/include/version_number.h"

#include <algorithm>
#include <charconv>
#include <iterator>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <system_error>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

namespace mmotd::semver {

struct version {
    int32_t major = 0;
    int32_t minor = 0;
    int32_t patch = 0;
    string prerelease;
    string build_number;

    static optional<version> from_string(string_view str);
    string to_string() const;
};

namespace detail {

inline string RemoveWhitespace(string input) {
    input.erase(remove_if(begin(input), end(input), boost::is_space()), end(input));
    return input;
}

inline string RemoveComments(string input) {
    auto comment_regex = regex(R"(//[^\n]*)");
    return regex_replace(input, comment_regex, "");
}

// since C++11 std::regex does not have a flag for VERBOSE mode we have created our own
inline string operator""_verbose(const char *str, size_t length) {
    return detail::RemoveWhitespace(detail::RemoveComments({str, str + length}));
}

int32_t FromString(string_view str) {
    auto result = int32_t{0};
    auto [ptr, ec] = std::from_chars(begin(str), end(str), result);
    if (ec != std::errc{}) {
        PLOG_ERROR << format("unable to convert {} into an integer, {}", str, make_error_code(ec).message());
    }
    return result;
}

} // namespace detail

optional<version> version::from_string(string_view str) {
    using detail::operator""_verbose;
    // https://semver.org/ is the source of regex: https://regex101.com/r/vkijKf/1/
    static const string REGEX_SEMVER_PATTERN = R"(
    ^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)                // major, minor, patch
      (?:-((?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)          // prerelease...
      (?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?     // followed by more prerelease...
      (?:\+([0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$)"_verbose; // build number

    const auto &pattern = REGEX_SEMVER_PATTERN;
    PLOG_INFO << format("pattern should now match verbose mode \"{}\"", pattern);
    auto semver_regex = regex(pattern);
    auto matches = std::cmatch{};
    if (!std::regex_match(begin(str), end(str), matches, semver_regex)) {
        PLOG_ERROR << format("version {} is not a semver compliant version string", str);
        return nullopt;
    }

    if (matches.size() < 3) {
        PLOG_ERROR << "semver regex succeeded but there were not 3 sub-matches (major, minor, patch)";
        return nullopt;
    }

    auto major = detail::FromString(string_view(matches.str(1)));
    auto minor = detail::FromString(string_view(matches.str(2)));
    auto patch = detail::FromString(string_view(matches.str(3)));
    auto prerelease = string{};
    if (matches.size() > 4) {
        prerelease = matches.str(4);
    }
    auto build_number = string{};
    if (matches.size() > 5) {
        build_number = matches.str(5);
    }
    auto semver_version = semver::version{major, minor, patch, prerelease, build_number};
    return make_optional(semver_version);
}

string version::to_string() const {
    auto version_str = format("{}.{}.{}", major, minor, patch);
    if (!prerelease.empty()) {
        version_str += format("-{}", prerelease);
    }
    if (!build_number.empty()) {
        version_str += format("+{}", build_number);
    }
    return version_str;
}

} // namespace mmotd::semver

namespace mmotd::version {

const Version &Version::Instance() {
    static auto version = Version(string_view{mmotd::version::MMOTD_VERSION});
    return version;
}

Version::Version(string_view str) : version_() {
    auto version_wrapper = mmotd::semver::version::from_string(str);
    if (!version_wrapper) {
        PLOG_ERROR << format("unable to create semver version from {} version string", str);
    }
    version_ = make_unique<mmotd::semver::version>(version_wrapper ? *version_wrapper : mmotd::semver::version{});
}

string Version::to_string() const {
    return version_ != nullptr ? version_->to_string() : string{};
}

int32_t Version::GetMajor() const {
    return version_ != nullptr ? version_->major : int32_t{0};
}

int32_t Version::GetMinor() const {
    return version_ != nullptr ? version_->minor : int32_t{0};
}

int32_t Version::GetPatch() const {
    return version_ != nullptr ? version_->patch : int32_t{0};
}

string Version::GetPrerelease() const {
    return version_ != nullptr ? version_->prerelease : string{};
}

string Version::GetBuildNumber() const {
    return version_ != nullptr ? version_->build_number : string{};
}

} // namespace mmotd::version
