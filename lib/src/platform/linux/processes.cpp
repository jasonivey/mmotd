// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__linux__)
#include "common/include/logging.h"
#include "lib/include/platform/processes.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

namespace fs = std::filesystem;
using namespace std;
using fmt::format;

namespace {

constexpr static const char *PROC_DIRECTORY = "/proc";

vector<string> GetProcessDirectories() {
    auto process_subdirs = vector<string>{};
    std::error_code ec;
    //auto flags = fs::directory_options::skip_permission_denied;
    for (auto &dir_entry : fs::directory_iterator(PROC_DIRECTORY, ec /*, flags*/)) {
        if (ec) {
            LOG_ERROR("error encountered while iterating {} directory, {}", PROC_DIRECTORY, ec.message());
            return vector<string>{};
        }

        auto path_entry = dir_entry.path();
        if (!dir_entry.is_directory()) {
            //LOG_DEBUG("ignoring {} since it is not a directory", path_entry.string());
            continue;
        }

        const auto &name = path_entry.stem().string();
        if (!all_of(begin(name), end(name), boost::is_digit())) {
            //LOG_DEBUG("ignoring {} since it is not all digits", path_entry.string());
            continue;
        }
        //LOG_DEBUG("adding process directory {}", path_entry.string());
        process_subdirs.push_back(path_entry.string());
    }

    if (ec) {
        LOG_ERROR("error encountered while iterating {} directory, {}", PROC_DIRECTORY, ec.message());
        return vector<string>{};
    }

    return process_subdirs;
}

optional<size_t> GetProcessDirectoriesCount() {
    auto process_subdirs = GetProcessDirectories();
    return process_subdirs.empty() ? nullopt : make_optional(process_subdirs.size());
}

} // namespace

namespace mmotd::platform {

optional<size_t> GetProcessCount() {
    return GetProcessDirectoriesCount();
}

} // namespace mmotd::platform
#endif
