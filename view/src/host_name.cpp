// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/host_name.h"

#include <algorithm>
#include <iterator>
#include <memory>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkHostNameProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_shared<mmotd::HostName>(); });

optional<string> mmotd::HostName::QueryInformation() {
    auto host_name_wrapper = ComputerInformation::Instance().GetInformation("system information");
    if (!host_name_wrapper) {
        PLOG_ERROR << "no host names were returned from posix system information";
        return nullopt;
    }
    auto values = (*host_name_wrapper);
    auto i =
        find_if(begin(values), end(values), [](const auto &value) { return boost::starts_with(value, "host name: "); });
    if (i == end(values)) {
        return nullopt;
    }

    const auto &hostname = (*i).substr(string{"host name: "}.size());
    return make_optional(hostname);
}

string mmotd::HostName::GetName() const {
    return "host name";
}
