// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/human_size.h"
#include "lib/include/computer_information.h"
#include "lib/include/file_system.h"

#include <filesystem>

#include <fmt/format.h>
#include <plog/Log.h>

#include <sys/sysctl.h>

using fmt::format;
using namespace std;
using mmotd::algorithm::string::to_human_size;

bool gLinkFileSystem = false;

namespace mmotd {

static const bool file_system_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::FileSystem>(); });

bool FileSystem::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        return GetFileSystemDetails();
    }
    return has_queried;
}

optional<mmotd::ComputerValues> FileSystem::GetInformation() const {
    return !file_system_details_.empty() ? make_optional(file_system_details_) : nullopt;
}

bool FileSystem::GetFileSystemDetails() {
    auto ec = error_code{};
    auto root_fs = std::filesystem::space("/", ec);
    if (ec) {
        PLOG_ERROR << format("getting fs::space on root file system, details: {}", ec.message());
        return false;
    }
    auto used = root_fs.capacity - root_fs.available;
    auto percent_used = (used * 100) / static_cast<double>(root_fs.capacity);
    file_system_details_.push_back(
        make_tuple("file system", format("Usage of /: {:.01f}% of {}", percent_used, to_human_size(root_fs.capacity))));
    file_system_details_.push_back(
        make_tuple("file system", format("capacity: {}, {}", to_human_size(root_fs.capacity), root_fs.capacity)));
    file_system_details_.push_back(
        make_tuple("file system", format("free: {}, {}", to_human_size(root_fs.free), root_fs.free)));
    file_system_details_.push_back(
        make_tuple("file system", format("available: {}, {}", to_human_size(root_fs.available), root_fs.available)));
    return true;
}

} // namespace mmotd
