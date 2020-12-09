// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/users_count.h"

#include <memory>
#include <regex>

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkUsersCountProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_shared<mmotd::UsersCount>(); });

optional<string> mmotd::UsersCount::QueryInformation() {
    auto user_sessions_wrapper = ComputerInformation::Instance().GetInformation("user session");
    if (!user_sessions_wrapper) {
        PLOG_INFO << "unable to find user sessions";
        return nullopt;
    }
    auto rgx = std::regex(" logged in (\\d+) times", std::regex_constants::ECMAScript);
    auto values = (*user_sessions_wrapper);
    for (auto value : values) {
        auto match = std::smatch{};
        if (!regex_search(value, match, rgx)) {
            continue;
        }
        if (match.size() > 0) {
            auto submatch = match[1];
            return make_optional(submatch.str());
        }
    }
    return nullopt;
}

string mmotd::UsersCount::GetName() const {
    return "users count";
}
