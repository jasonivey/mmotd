// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/logging.h"
#include "common/include/special_files.h"
#include "common/include/user_information.h"

#include <filesystem>
#include <system_error>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <unistd.h>

using fmt::format;
namespace fs = std::filesystem;
using namespace std;

namespace {

fs::path FindProjectRootDirectory() {
#if defined(DEBUG)
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
#else
    return fs::path{};
#endif
}

// %LocalAppData% -- Windows
// ~/Library/Application Support/ -- macOS
// $XDG_CONFIG_HOME -- Linux
fs::path FindFileImpl(string_view file_name) {
    auto config_path = fs::path{};
    auto ec = error_code{};

    // if running a debug build, try using the in-source version
    if (auto project_root = FindProjectRootDirectory(); !project_root.empty()) {
        config_path = project_root / "config" / file_name;
        LOG_VERBOSE("checking {} for {}", quoted(config_path.parent_path().string()), quoted(file_name));
        if (fs::exists(config_path, ec) && !ec) {
            LOG_VERBOSE("found {} in {}", quoted(file_name), quoted(config_path.parent_path().string()));
            return fs::canonical(config_path);
        }
    }

    auto user_info = mmotd::core::GetUserInformation();
    if (!empty(user_info.home_directory) && fs::exists(user_info.home_directory, ec) && !ec) {
        // check for location in various config directories
        config_path = fs::canonical(fs::path(user_info.home_directory)) / ".config" / "mmotd" / file_name;
        LOG_VERBOSE("checking {} for {}", quoted(config_path.parent_path().string()), quoted(file_name));
        if (fs::exists(config_path, ec) && !ec) {
            LOG_VERBOSE("found {} in {}", quoted(file_name), quoted(config_path.parent_path().string()));
            return fs::canonical(config_path);
        }
        // lastly, check whether it is in the root of the home directory
        config_path = fs::canonical(fs::path(user_info.home_directory)) / file_name;
        LOG_VERBOSE("checking {} for {}", quoted(config_path.parent_path().string()), quoted(file_name));
        if (fs::exists(config_path, ec) && !ec) {
            LOG_VERBOSE("found {} in {}", quoted(file_name), quoted(config_path.parent_path().string()));
            return fs::canonical(config_path);
        }
    }
    LOG_VERBOSE("unable to find {} anywhere in the predefined locations", quoted(file_name));
    return fs::path{};
}

auto IsStdoutTtyImpl() -> bool {
    auto is_stdout_tty = isatty(STDOUT_FILENO) != 0;
    LOG_VERBOSE("stdout is{} a TTY", is_stdout_tty ? "" : " not");
    return is_stdout_tty;
}

} // namespace

namespace mmotd::core::special_files {

auto IsStdoutTty() -> bool {
    static const auto is_stdout_tty = IsStdoutTtyImpl();
    return is_stdout_tty;
}

fs::path FindFileInDefaultLocations(string_view file_name) {
    return FindFileImpl(file_name);
}

} // namespace mmotd::core::special_files
