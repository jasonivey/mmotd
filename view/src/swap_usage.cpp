// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/swap_usage.h"

#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/range/iterator.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkSwapUsageProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_shared<mmotd::SwapUsage>(); });

optional<string> mmotd::SwapUsage::QueryInformation() {
    auto swap_usage_wrapper = ComputerInformation::Instance().GetInformation("swap usage");
    if (!swap_usage_wrapper) {
        PLOG_INFO << "unable to find swap usage information";
        return nullopt;
    }
    auto values = (*swap_usage_wrapper);
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

string mmotd::SwapUsage::GetName() const {
    return "swap usage";
}
