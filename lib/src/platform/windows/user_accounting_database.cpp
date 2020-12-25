// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include "lib/include/platform/user_accounting_database.h"

#include <string>
#include <vector>

using namespace std;

namespace {

mmotd::platform::UserAccountEntries GetUserAccountEntriesImpl() {
    return mmotd::platform::UserAccountEntries{};
}

mmotd::platform::UserInformation GetUserInformationImpl() {
    return mmotd::platform::UserInformation{};
}

namespace mmotd::platform {

UserAccountEntry UserAccountEntry::from_utmp(const utmp &db) {
    return UserAccountEntry{};
}

UserAccountEntries GetUserAccountEnteries() {
    static auto user_account_enteries = GetUserAccountEntriesImpl();
    return user_account_enteries;
}

UserInformation UserInformation::from_passwd(const passwd &pw) {
    return UserInformation{};
}

const UserInformation &GetUserInformation() {
    static auto user_information = GetUserInformationImpl();
    return user_information;
}

} // namespace mmotd::platform

#endif
