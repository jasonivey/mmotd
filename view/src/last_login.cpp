// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/last_login.h"

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
    auto combined_value = string{};
    for (auto value : last_login_values) {
        combined_value += combined_value.empty() ? value : format(", {}", value);
    }
    return make_optional(combined_value);
}

string mmotd::LastLogin::GetName() const {
    return "last login";
}
