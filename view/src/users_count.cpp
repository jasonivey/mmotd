// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/users_count.h"

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkUsersCountProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::UsersCount>(); });

optional<string> mmotd::UsersCount::QueryInformation() {
    auto user_sessions_wrapper = ComputerInformation::Instance().GetInformation("user session");
    if (!user_sessions_wrapper) {
        PLOG_INFO << "unable to find user sessions";
        return nullopt;
    }
    auto values = (*user_sessions_wrapper);
    auto combined_value = string{};
    for (auto value : values) {
        combined_value += format("{}{}", combined_value.empty() ? "" : ", ", value);
    }
    return make_optional(combined_value);
}

string mmotd::UsersCount::GetName() const {
    return "users count";
}
