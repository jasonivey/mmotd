// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/system_load.h"

#include <memory>
#include <string_view>

#include <fmt/format.h>
#include <plog/Log.h>

using namespace std::literals;
using fmt::format;
using namespace std;

bool gLinkSystemLoadProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_shared<mmotd::SystemLoad>(); });

optional<string> mmotd::SystemLoad::QueryInformation() {
    auto load_average_wrapper = ComputerInformation::Instance().GetInformation("load average");
    if (!load_average_wrapper) {
        PLOG_INFO << "unable to find load average";
        return nullopt;
    }
    auto values = (*load_average_wrapper);
    if (values.size() > 1) {
        auto combined_value = string{};
        for (auto value : values) {
            combined_value += format("{}{}", combined_value.empty() ? "" : ", ", value);
        }
        return make_optional(combined_value);
    } else {
        auto str = values.front();
        auto index = str.find("system load average: ");
        if (index == string::npos) {
            return make_optional(str);
        }
        str = str.substr("system load average: "sv.size());
        return make_optional(str);
    }
}

string mmotd::SystemLoad::GetName() const {
    return "system load";
}
