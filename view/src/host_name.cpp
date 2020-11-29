// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/host_name.h"

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkHostNameProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::HostName>(); });

optional<string> mmotd::HostName::QueryInformation() {
    auto host_name_info = ComputerInformation::Instance().GetInformation("host name");
    if (!host_name_info) {
        PLOG_INFO << "no host names were returned from computer information api";
        return nullopt;
    } else if ((*host_name_info).size() != 1) {
        PLOG_INFO << format("a host name was returned but the list contained {} items", (*host_name_info).size());
        return nullopt;
    } else if ((*host_name_info).front().empty()) {
        PLOG_ERROR << "the host name was returned but it was empty";
        return nullopt;
    }
    auto value = (*host_name_info).front();
    return make_optional(value);
}

string mmotd::HostName::GetName() const {
    return "host name";
}
