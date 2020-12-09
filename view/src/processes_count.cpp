// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/processes_count.h"

#include <memory>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkProcessesCountProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_shared<mmotd::ProcessesCount>(); });

optional<string> mmotd::ProcessesCount::QueryInformation() {
    auto processes_count_wrapper = ComputerInformation::Instance().GetInformation("processes");
    if (!processes_count_wrapper) {
        PLOG_ERROR << "processes count was not returned";
        return nullopt;
    }
    auto values = (*processes_count_wrapper);
    for (const auto &value : values) {
        if (boost::starts_with(value, "count: ")) {
            return make_optional(value.substr(string{"count: "}.size()));
        }
    }
    return nullopt;
}

string mmotd::ProcessesCount::GetName() const {
    return "processes count";
}
