// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/computer_name.h"

#include <plog/Log.h>

using namespace std::string_view_literals;
using namespace std;

bool gLinkComputerNameProvider = false;

static const bool computer_name_factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::ComputerName>(); });

optional<string> mmotd::ComputerName::QueryInformation() {
#if 0
    auto &info = PosixSystemInformation::Instance();
    if (!info.QueryInformation()) {
        PLOG_ERROR << "while attempting to query posix system information";
        return nullopt;
    }
    const auto &details = info.GetInformation();
    if (!details) {
        PLOG_ERROR << "while attempting to retreive posix system information";
        return nullopt;
    }
    for (const auto &values : *details) {
        const auto [name, value] = values;
        if (name == GetName()) {
            auto index = value.find(".");
            if (index != string::npos) {
                return make_optional(value.substr(0, index));
            } else {
                return make_optional(value);
            }
        }
    }
#endif
    auto computer_name = ComputerInformation::Instance().GetInformation("host name"sv);
    if (!computer_name || (*computer_name).size() != 1) {
        PLOG_INFO << "no host names were returned from posix system information";
    }
    auto value = (*computer_name).front();
    auto index = value.find(".");
    if (index != string::npos) {
        return make_optional(value.substr(0, index));
    } else if (value.empty()) {
        return nullopt;
    } else {
        return make_optional(value);
    }
}

string mmotd::ComputerName::GetName() const {
    return "host name";
}
