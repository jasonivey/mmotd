// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/swap_usage.h"

#include <memory>

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
    if (values.size() == 1) {
        return make_optional(values.front());
    }
    auto combined_value = string{};
    for (auto value : values) {
        combined_value += format("{}{}", combined_value.empty() ? "" : ", ", value);
    }
    return make_optional(combined_value);
}

string mmotd::SwapUsage::GetName() const {
    return "swap usage";
}
