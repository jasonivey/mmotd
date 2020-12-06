// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/computer_name.h"

#include <algorithm>
#include <iterator>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkComputerNameProvider = false;

static const bool computer_name_factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::ComputerName>(); });

optional<string> mmotd::ComputerName::QueryInformation() {
    auto system_info_wrapper = ComputerInformation::Instance().GetInformation("system information");
    if (!system_info_wrapper) {
        PLOG_ERROR << "no host names were returned from posix system information";
        return nullopt;
    }
    auto system_infos = (*system_info_wrapper);
    auto i = find_if(begin(system_infos), end(system_infos), [](const auto &value) {
        return boost::starts_with(value, "host name: ");
    });

    if (i == end(system_infos)) {
        return nullopt;
    }

    const auto &hostname = (*i).substr(string{"host name: "}.size());
    auto index = hostname.find(".");
    if (index == string::npos) {
        return make_optional(hostname);
    }
    return make_optional(hostname.substr(0, index));
}

string mmotd::ComputerName::GetName() const {
    return "computer name";
}
