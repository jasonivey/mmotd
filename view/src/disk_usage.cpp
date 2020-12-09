// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/disk_usage.h"

#include <memory>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkDiskUsageProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_shared<mmotd::DiskUsage>(); });

optional<string> mmotd::DiskUsage::QueryInformation() {
    auto fs_usages_wrapper = ComputerInformation::Instance().GetInformation("file system");
    if (!fs_usages_wrapper) {
        PLOG_ERROR << "no file system usages were returned from system information";
        return nullopt;
    }
    auto fs_usages = fs_usages_wrapper.value();
    auto repr = string{};
    for (auto fs_usage : fs_usages) {
        auto rng = boost::find_first(fs_usage, "Usage of /: ");
        if (!rng) {
            continue;
        }
        auto value = string{rng.end(), fs_usage.end()};
        return make_optional(value);
    }
    return nullopt;
}

string mmotd::DiskUsage::GetName() const {
    return "disk usage";
}
