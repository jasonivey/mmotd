// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/computer_name.h"

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkComputerNameProvider = false;

static const bool computer_name_factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::ComputerName>(); });

optional<string> mmotd::ComputerName::QueryInformation() {
    auto host_name = ComputerInformation::Instance().GetInformation("host name");
    if (!host_name) {
        PLOG_ERROR << "no host names were returned from posix system information";
        return nullopt;
    } else if ((*host_name).size() != 1) {
        PLOG_ERROR << format("the computer name was returned but there were {} values", (*host_name).size());
        return nullopt;
    } else if ((*host_name).front().empty()) {
        PLOG_ERROR << "the computer name was returned but it was empty";
        return nullopt;
    }
    auto value = (*host_name).front();
    auto index = value.find(".");
    if (index != string::npos) {
        return make_optional(value.substr(0, index));
    } else {
        return make_optional(value);
    }
}

string mmotd::ComputerName::GetName() const {
    return "host name";
}
