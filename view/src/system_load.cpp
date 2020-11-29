// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/system_load.h"

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkSystemLoadProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::SystemLoad>(); });

optional<string> mmotd::SystemLoad::QueryInformation() {
    auto load_average_wrapper = ComputerInformation::Instance().GetInformation("load average");
    if (!load_average_wrapper) {
        PLOG_INFO << "unable to find load average";
        return nullopt;
    } else if ((*load_average_wrapper).size() != 1) {
        PLOG_INFO << format("found load average but there were {} values returned", (*load_average_wrapper).size());
        return nullopt;
    } else if ((*load_average_wrapper).front().empty()) {
        PLOG_ERROR << "the load average was returned but it was empty";
        return nullopt;
    }
    auto value = (*load_average_wrapper).front();
    return make_optional(value);
}

string mmotd::SystemLoad::GetName() const {
    return "system load";
}
