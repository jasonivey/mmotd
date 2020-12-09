// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_boot_time.h"
#include "view/include/computer_information_provider_factory.h"

#include <memory>

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkComputerBootTimeProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_shared<mmotd::ComputerBootTime>(); });

optional<string> mmotd::ComputerBootTime::QueryInformation() {
    auto boot_time_wrapper = ComputerInformation::Instance().GetInformation("boot time");
    if (!boot_time_wrapper || (*boot_time_wrapper).empty()) {
        PLOG_ERROR << "unable to find computer boot time";
        return nullopt;
    } else if ((*boot_time_wrapper).size() != 1) {
        PLOG_ERROR << format("able to query the computer boot time successfully but {} items were returned",
                             (*boot_time_wrapper).size());
        return nullopt;
    } else if ((*boot_time_wrapper).front().empty()) {
        PLOG_ERROR << "able to query the computer boot time successfully but the value was empty";
        return nullopt;
    }
    return make_optional((*boot_time_wrapper).front());
}

string mmotd::ComputerBootTime::GetName() const {
    return "boot time";
}
