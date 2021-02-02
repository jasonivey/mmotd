// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/platform/user_accounting_database.h"
#include "lib/include/users_logged_in.h"

#include <algorithm>
#include <iterator>

#include <fmt/format.h>
#include <plog/Log.h>

using namespace mmotd::platform;
using fmt::format;
using namespace std;

bool gLinkUsersLoggedIn = false;

namespace mmotd::information {

static const bool users_logged_in_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::UsersLoggedIn>(); });

bool UsersLoggedIn::FindInformation() {
    if (auto logged_in = GetUsersLoggedIn(); !logged_in.empty()) {
        auto logged = GetInfoTemplate(InformationId::ID_LOGGED_IN_USER_LOGGED_IN);
        logged.information = logged_in;
        AddInformation(logged);
        return true;
    } else {
        return false;
    }
}

string UsersLoggedIn::GetUsersLoggedIn() {
    using namespace mmotd::platform::user_account_database;
    using namespace mmotd::platform::user;

    auto user_information = GetUserInformation();
    auto user_account_enteries = GetDbEntries<ENTRY_TYPE::User>();
    if (user_information.empty() || user_account_enteries.empty()) {
        PLOG_ERROR << format("user information empty: {}, user account entry size: {}",
                             user_information.empty(),
                             user_account_enteries.size());
        // should never happen
        return string{};
    }

    const auto &username = user_information.username;
    const user_account_database::DbEntry *user_account_entry_ptr = nullptr;
    auto user_count = count_if(begin(user_account_enteries),
                               end(user_account_enteries),
                               [&username, &user_account_entry_ptr](const auto &user_account_entry) {
                                   if (username == user_account_entry.user) {
                                       if (user_account_entry_ptr == nullptr) {
                                           user_account_entry_ptr = &user_account_entry;
                                       }
                                       return true;
                                   }
                                   return false;
                               });

    if (user_count == 0 || user_account_entry_ptr == nullptr) {
        PLOG_ERROR << format("user count: {}, user account ptr: {}", user_count, fmt::ptr(user_account_entry_ptr));
        // should never happen
        return string{};
    }

    const auto &user_account_entry = *user_account_entry_ptr;
    auto session_str = format("{} logged in {} time{}", user_account_entry.user, user_count, user_count > 1 ? "s" : "");
    if (!user_account_entry.hostname.empty()) {
        session_str += format(" from {}", user_account_entry.hostname);
    }
    return session_str;
}

} // namespace mmotd::information
