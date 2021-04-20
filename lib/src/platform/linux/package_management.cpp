// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/platform/package_management.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

namespace fs = std::filesystem;
using namespace std;
using fmt::format;

constexpr static const char *UPDATES_AVAILABLE_FILE = "/var/lib/update-notifier/updates-available";

namespace {

string ReadFile(const char *path_str) {
    auto path = fs::path(path_str);
    std::error_code ec;
    if (auto file_exists = fs::exists(path, ec); !file_exists || ec) {
        PLOG_ERROR << format(FMT_STRING("package management file does not exist {}"), UPDATES_AVAILABLE_FILE);
        return string{};
    }
    auto strm = ifstream(path);
    if (!strm) {
        PLOG_ERROR << format(FMT_STRING("unable to open package management file {}"), path.string());
        return string{};
    }
    auto lines = vector<string>{};
    for (auto line = string{}; getline(strm, line);) {
        boost::trim(line);
        if (!empty(line)) {
            lines.push_back(line);
        }
    }
    return format(FMT_STRING("{}"), fmt::join(lines, "\n"));
}

} // namespace

namespace mmotd::platform {

string GetPackageManagementUpdate() {
    return ReadFile(UPDATES_AVAILABLE_FILE);
}

} // namespace mmotd::platform
