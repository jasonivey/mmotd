// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
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
    auto memory_usage_wrapper = ComputerInformation::Instance().GetInformation("memory usage");
    if (!memory_usage_wrapper) {
        PLOG_INFO << "unable to find memory usage";
        return nullopt;
    }
    auto values = (*memory_usage_wrapper);
    auto combined_value = string{};
    for (auto value : values) {
        auto rng = boost::find_first(value, "percent: ");
        if (rng) {
            return make_optional(string(rng.end(), value.end()));
        }
    }
    return nullopt;
}

string mmotd::MemoryUsage::GetName() const {
    return "memory usage";
}
