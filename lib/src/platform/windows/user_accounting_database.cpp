// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(_WIN32)
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

UserAccountEntries GetUserAccountEnteries() {
    static auto user_account_enteries = GetUserAccountEntriesImpl();
    return user_account_enteries;
}

const UserInformation &GetUserInformation() {
    static auto user_information = GetUserInformationImpl();
    return user_information;
}

} // namespace mmotd::platform
#endif
