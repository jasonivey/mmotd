// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/platform/user_accounting_database.h"
#include "lib/include/users_logged_in.h"

#include <algorithm>
#include <iterator>

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkUsersLoggedIn = false;

namespace mmotd {

static const bool users_logged_in_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::UsersLoggedIn>(); });

bool UsersLoggedIn::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        return GetUsersLoggedIn();
    }
    return has_queried;
}

optional<mmotd::ComputerValues> UsersLoggedIn::GetInformation() const {
    return !details_.empty() ? make_optional(details_) : nullopt;
}

bool UsersLoggedIn::GetUsersLoggedIn() {
    auto user_information = mmotd::platform::GetUserInformation();
    auto user_account_enteries = mmotd::platform::GetUserAccountEntries();
    if (user_information.empty() || user_account_enteries.empty()) {
        // should never happen
        return false;
    }

    const auto &username = user_information.username;
    const mmotd::platform::UserAccountEntry *user_account_entry_ptr = nullptr;
    auto user_count = count_if(begin(user_account_enteries),
                               end(user_account_enteries),
                               [&username, &user_account_entry_ptr](const auto &user_account_entry) {
                                   if (user_account_entry.is_user() && username == user_account_entry.user) {
                                       user_account_entry_ptr =
                                           user_account_entry_ptr == nullptr ? &user_account_entry : nullptr;
                                       return true;
                                   }
                                   return false;
                               });

    if (user_count == 0 || user_account_entry_ptr == nullptr) {
        // should never happen
        return false;
    }

    const auto &user_account_entry = *user_account_entry_ptr;
    auto session_str = format("{} logged in {} time{}", user_account_entry.user, user_count, user_count > 1 ? "s" : "");
    if (!user_account_entry.hostname.empty()) {
        session_str += format(" from {}", user_account_entry.hostname);
    }
    details_.push_back(make_tuple("user session", session_str));
    return !details_.empty();
}

} // namespace mmotd
