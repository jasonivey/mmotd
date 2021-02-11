// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/human_size.h"
#include "lib/include/computer_information.h"
#include "lib/include/file_system.h"

#include <filesystem>

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;
using mmotd::algorithm::string::to_human_size;

bool gLinkFileSystem = false;

namespace mmotd::information {

static const bool file_system_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::FileSystem>(); });

bool FileSystem::FindInformation() {
    auto ec = error_code{};
    auto root_fs = std::filesystem::space("/", ec);
    if (ec) {
        PLOG_ERROR << format("getting fs::space on root file system, details: {}", ec.message());
        return false;
    }

    auto usage = GetInfoTemplate(InformationId::ID_FILE_SYSTEM_USAGE);
    usage.SetName("Usage of /");
    auto percent_used = ((root_fs.capacity - root_fs.available) * 100) / static_cast<double>(root_fs.capacity);
    usage.SetValue(percent_used, to_human_size(root_fs.capacity));
    AddInformation(usage);

    auto capacity = GetInfoTemplate(InformationId::ID_FILE_SYSTEM_CAPACITY);
    capacity.SetValue(root_fs.capacity);
    AddInformation(capacity);

    auto free = GetInfoTemplate(InformationId::ID_FILE_SYSTEM_FREE);
    free.SetValue(root_fs.free);
    AddInformation(free);

    auto available = GetInfoTemplate(InformationId::ID_FILE_SYSTEM_AVAILABLE);
    available.SetValue(root_fs.available);
    AddInformation(available);

    return true;
}

} // namespace mmotd::information
