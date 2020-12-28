// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/human_size.h"
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/memory_usage.h"

#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/range/iterator.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkMemoryUsageProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_shared<mmotd::MemoryUsage>(); });

optional<string> mmotd::MemoryUsage::QueryInformation() {
    using mmotd::algorithm::string::to_human_size;

    auto memory_usage_wrapper = ComputerInformation::Instance().GetInformation("memory usage");
    if (!memory_usage_wrapper) {
        PLOG_INFO << "unable to find memory usage";
        return nullopt;
    }
    auto values = (*memory_usage_wrapper);
    auto percent_used = string{};
    auto total = string{};
    for (auto value : values) {
        if (auto rng = boost::find_first(value, "percent: "); rng) {
            percent_used = string(rng.end(), value.end());
        }
        if (auto rng = boost::find_first(value, "total: "); rng) {
            total = string(rng.end(), value.end());
        }
        if (!percent_used.empty() && !total.empty()) {
            break;
        }
    }
    if (!percent_used.empty() && !total.empty()) {
        return make_optional(format("{} of {}", percent_used, total));
    } else if (!percent_used.empty()) {
        return make_optional(percent_used);
    } else {
        return nullopt;
    }
}

string mmotd::MemoryUsage::GetName() const {
    return "memory usage";
}
