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
constexpr static const char *REBOOT_REQUIRED_FILE = "/var/run/reboot-required";

namespace {

string ReadFile(const char *path_str) {
    PLOG_VERBOSE << "reading file" << path_str;
    auto path = fs::path(path_str);
    std::error_code ec;
    if (auto file_exists = fs::exists(path, ec); !file_exists || ec) {
        PLOG_ERROR << format(FMT_STRING("package management file does not exist {}"), path.string());
        return string{};
    }
    auto input = ifstream(path);
    if (!input) {
        PLOG_ERROR << format(FMT_STRING("unable to open package management file {}"), path.string());
        return string{};
    }
    auto lines = vector<string>{};
    for (auto line = string{}; getline(input, line);) {
        boost::trim(line);
        if (!empty(line)) {
            lines.push_back(line);
        }
    }
    return format(FMT_STRING("{}"), fmt::join(lines, "\n"));
}

} // namespace

namespace mmotd::platform::package_management {

string GetUpdateDetails() {
    PLOG_VERBOSE << "[linux] getting package management update details";
    auto update_details = ReadFile(UPDATES_AVAILABLE_FILE);
    PLOG_VERBOSE << format(FMT_STRING("[linux] returning from package management update details: {}"), update_details);
    return update_details;
}

string GetRebootRequired() {
    PLOG_VERBOSE << "[linux] getting package management reboot required";
    auto reboot_required = ReadFile(REBOOT_REQUIRED_FILE);
    PLOG_VERBOSE << format(FMT_STRING("[linux] returning from package management reboot required: {}"),
                           reboot_required);
    return reboot_required;
}

} // namespace mmotd::platform::package_management
