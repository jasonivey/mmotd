// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/logging.h"
#include "common/include/special_files.h"
#include "common/include/user_information.h"

#include <filesystem>
#include <iomanip>
#include <iterator>
#include <regex>
#include <string>
#include <string_view>
#include <system_error>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <unistd.h>

using fmt::format;
namespace fs = std::filesystem;
using namespace std;

namespace {

#if defined(DEBUG)
fs::path FindProjectConfigDirectory() {
    // Find the '<project root>/config' directory
    auto project_root = mmotd::core::special_files::FindProjectRootDirectory();
    if (empty(project_root)) {
        return fs::path{};
    }
    auto ec = error_code{};
    auto config_dir = project_root / "config" / "";
    if (auto stat = fs::status(config_dir, ec); ec || !fs::is_directory(stat)) {
        return fs::path{};
    }
    auto config_dir_abs = fs::canonical(config_dir, ec);
    return ec ? fs::path{} : config_dir_abs;
}
#else
fs::path FindProjectConfigDirectory() {
    return fs::path{};
}
#endif

string StripEnvironmentVariable(string input) {
    using boost::trim_right_copy_if, boost::trim_left_copy_if, boost::is_any_of;
    PRECONDITIONS(!empty(input), "unable to strip an environment variable of $ or ${} if its empty");
    return trim_right_copy_if(trim_left_copy_if(input, is_any_of("${")), is_any_of("}"));
}

#if defined(__APPLE__) || defined(__linux__)
vector<fs::path> GetDefaultLocationsUnixCommon() {
    // $HOME & $XDG_CONFIG_HOME
    auto default_locations = vector<fs::path>{};
    auto user_info = mmotd::core::GetUserInformation();
    auto ec = error_code{};
    if (empty(user_info.home_directory) || !fs::exists(user_info.home_directory, ec) || ec) {
        LOG_VERBOSE("home directory either not found or does not exist");
        return default_locations;
    }
    auto home_dir = fs::path(user_info.home_directory);
    auto dot_config_dir = home_dir / ".config" / "mmotd" / "";
    if (auto dot_config_dir_abs = fs::weakly_canonical(dot_config_dir, ec); !ec) {
        default_locations.emplace_back(dot_config_dir_abs);
    }
    if (auto home_dir_abs = fs::canonical(home_dir / "", ec); !ec) {
        default_locations.emplace_back(home_dir_abs);
    }
    return default_locations;
}
#endif

#if defined(__APPLE__)
vector<fs::path> GetDefaultLocationsPlatform() {
    // Need to add $HOME/Library/Application Support/
    return GetDefaultLocationsUnixCommon();
}
#elif defined(__linux__)
vector<fs::path> GetDefaultLocationsPlatform() {
    return GetDefaultLocationsUnixCommon();
}
#elif defined(_WIN32)
vector<fs::path> GetDefaultLocationsPlatform() {
    // Need to add %LocalAppData%
    return vector<fs::path>{};
}
#endif

vector<fs::path> GetDefaultLocationsImpl() {
    auto default_locations = vector<fs::path>{};

    auto project_config_dir = FindProjectConfigDirectory();
    if (!empty(project_config_dir)) {
        default_locations.push_back(project_config_dir);
    }

    auto platform_default_locations = GetDefaultLocationsPlatform();
    default_locations.insert(end(default_locations),
                             begin(platform_default_locations),
                             end(platform_default_locations));

    return default_locations;
}

auto IsStdoutTtyImpl() -> bool {
    auto is_stdout_tty = isatty(STDOUT_FILENO) != 0;
    LOG_VERBOSE("stdout is{} a TTY", is_stdout_tty ? "" : " not");
    return is_stdout_tty;
}

} // namespace

namespace mmotd::core::special_files {

fs::path FindProjectRootDirectory() {
    auto dir_path = fs::current_path();
    auto ec = error_code{};
    auto finished = false;
    while (!finished) {
        auto git_dir = dir_path / ".git";
        if (auto stat = fs::status(git_dir, ec); !ec && fs::is_directory(stat)) {
            dir_path = fs::canonical(dir_path);
            finished = true;
        } else {
            auto parent_dir = dir_path.parent_path();
            auto found_root = fs::equivalent(dir_path, parent_dir, ec);
            if (ec) {
                LOG_ERROR("filesystem error {}: {}", ec.value(), ec.message());
                dir_path = fs::path{};
                finished = true;
            } else if (found_root) {
                LOG_INFO("unable to find .git directory within {}", fs::current_path());
                dir_path = fs::path{};
                finished = true;
            } else {
                dir_path = parent_dir;
            }
        }
    }
    return dir_path;
}

string GetEnvironmentValue(string variable_name) {
    if (empty(variable_name)) {
        return string{};
    }
    auto variable_name_stripped = StripEnvironmentVariable(variable_name);
    auto *variable_value = getenv(data(variable_name_stripped));
    return variable_value != nullptr ? string{variable_value} : string{};
}

string ExpandEnvironmentVariables(string input) {
    if (empty(input)) {
        return string{};
    }
    static auto env_pattern = std::regex{R"((\$\{[^}]+\})|(\$[\w\d_]+))"};
    auto envvar_begin = std::sregex_iterator(begin(input), end(input), env_pattern);
    auto envvar_end = std::sregex_iterator();
    auto output = string{};
    auto index = std::smatch::difference_type{0};
    for (std::sregex_iterator i = envvar_begin; i != envvar_end; ++i) {
        auto match = *i;
        if (match.position() > index) {
            output += input.substr(index, match.position() - index);
            index = static_cast<std::smatch::difference_type>(match.position() + size(match.str()));
        }

        output += GetEnvironmentValue(match.str());

        if (std::distance(i, envvar_end) == 1) {
            output += match.suffix().str();
        }
    }
    return envvar_begin == envvar_end ? input : output;
}

auto IsStdoutTty() -> bool {
    static const auto is_stdout_tty = IsStdoutTtyImpl();
    return is_stdout_tty;
}

vector<fs::path> GetDefaultLocations() {
    return GetDefaultLocationsImpl();
}

string GetDefaultLocationsStr() {
    const auto &default_locations = GetDefaultLocations();
    auto default_locations_str = string{};
    for (auto i = begin(default_locations); i != end(default_locations); ++i) {
        auto dir_path = (*i).string();
        if (!empty(dir_path) && dir_path.back() != fs::path::preferred_separator) {
            dir_path.push_back(fs::path::preferred_separator);
        }
        auto first = distance(begin(default_locations), i) == 0;
        auto last = distance(i, end(default_locations)) == 1;
        auto middle = !first && !last;
        if (first || (first && last)) {
            default_locations_str += format(FMT_STRING("{}"), quoted(dir_path));
        } else if (middle) {
            default_locations_str += format(FMT_STRING(", {}"), quoted(dir_path));
        } else if (last) {
            default_locations_str += format(FMT_STRING(" and {}"), quoted(dir_path));
        }
    }
    return default_locations_str;
}

fs::path FindFileInDefaultLocations(string_view file_name) {
    return FindFileInDefaultLocations(fs::path{file_name});
}

fs::path FindFileInDefaultLocations(string file_name) {
    return FindFileInDefaultLocations(fs::path{file_name});
}

fs::path FindFileInDefaultLocations(fs::path input_file_path) {
    const auto &default_locations = GetDefaultLocations();
    for (const auto &dir_path : default_locations) {
        auto file_path = dir_path / input_file_path;
        auto ec = error_code{};
        if (fs::exists(file_path, ec) && !ec) {
            if (auto file_path_abs = fs::canonical(file_path, ec); !ec) {
                LOG_VERBOSE("found {} in directory: {}", quoted(input_file_path.string()), quoted(dir_path.string()));
                return file_path_abs;
            }
        }
        LOG_VERBOSE("unable to find {} in directory: {}", quoted(input_file_path.string()), quoted(dir_path.string()));
    }
    LOG_VERBOSE("unable to find {} in any default locations", quoted(input_file_path.string()));
    return fs::path{};
}

} // namespace mmotd::core::special_files
