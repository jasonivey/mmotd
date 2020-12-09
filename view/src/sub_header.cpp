// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/sub_header.h"

#include <memory>

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkSubHeaderProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_shared<mmotd::SubHeader>(); });

optional<string> mmotd::SubHeader::QueryInformation() {
    auto weather_wrapper = ComputerInformation::Instance().GetInformation("weather");
    if (!weather_wrapper) {
        PLOG_INFO << "unable to find weather report";
        return nullopt;
    }
    auto values = (*weather_wrapper);
    auto combined_value = string{};
    for (auto value : values) {
        combined_value += format("{}{}", combined_value.empty() ? "" : " ", value);
    }
    return make_optional(combined_value);
}

string mmotd::SubHeader::GetName() const {
    return "sub header";
}
