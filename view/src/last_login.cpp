// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/last_login.h"

#include <array>
#include <string>

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkLastLoginProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::LastLogin>(); });

optional<string> mmotd::LastLogin::QueryInformation() {
    auto last_login_wrapper = ComputerInformation::Instance().GetInformation("last login");
    if (!last_login_wrapper || (*last_login_wrapper).empty()) {
        PLOG_INFO << "no last login information was returned from computer information api";
        return nullopt;
    }
    auto last_login_values = last_login_wrapper.value();
    PLOG_INFO << format("last login returned {} items", last_login_values.size());
    auto values = std::array<string, 3>{};
    for (auto value : last_login_values) {
        PLOG_INFO << format("last login item: {}", value);
        if (value.find("Log in: ") == 0) {
            values[1].assign(value);
        } else if (value.find("Log out: ") == 0) {
            values[2].assign(value);
        } else {
            values[0].assign(value);
        }
    }
    auto combined_value = string{};
    for (auto value : values) {
        combined_value += value + ", ";
    }
    return make_optional(combined_value.substr(0, combined_value.size() - 2));
}

string mmotd::LastLogin::GetName() const {
    return "last login";
}
