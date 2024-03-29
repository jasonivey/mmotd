// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__APPLE__)
#include "lib/include/platform/user_accounting_database.h"

#include "common/include/chrono_io.h"
#include "common/include/logging.h"
#include "common/include/posix_error.h"

#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <locale>
#include <string>
#include <vector>

#include <boost/asio/ip/address.hpp>
#include <fmt/format.h>
#include <scope_guard.hpp>

#include <pwd.h>
#include <unistd.h>
#include <utmpx.h>

using boost::asio::ip::make_address;
using ip_address = boost::asio::ip::address;
namespace pe = mmotd::error::posix_error;
using fmt::format;
using namespace std;

using namespace mmotd::platform::user_account_database;

namespace {

DbEntries GetUserAccountEntriesImpl() {
    // resets the database, so that the next getutxent() call will get the first entry
    setutxent();

    // auto close the database however we leave this function
    auto endutxent_closer = sg::make_scope_guard([]() noexcept { endutxent(); });

    auto entries = DbEntries{};

    // read the next entry from the database; if the database was not yet open, it also opens it
    auto i = size_t{0};
    for (const utmpx *utmpx_ptr = getutxent(); utmpx_ptr != nullptr; utmpx_ptr = getutxent(), ++i) {
        auto ut_type_str = DbEntry::entry_type_to_string(utmpx_ptr->ut_type);
        LOG_VERBOSE("iteration #{}, type: {}, utmpx *: {}", i + 1, ut_type_str, fmt::ptr(utmpx_ptr));

        auto entry = DbEntry::from_utmpx(*utmpx_ptr);
        if (!entry.empty()) {
            entries.push_back(entry);
            LOG_VERBOSE("adding user account entry #{}", entries.size());
        }
    }

    LOG_VERBOSE("returning {} user account entries", entries.size());
    return entries;
}

string GetUtmpxId(const utmpx &db) {
    auto id = string{};
    const auto &loc = std::locale();
    for (auto i = size_t{0}; i < size_t{_UTX_IDSIZE}; ++i) {
        if (std::isprint(db.ut_id[i], loc)) {
            id.push_back(db.ut_id[i]);
        }
    }
    return id;
}

} // namespace

namespace mmotd::platform::user_account_database {

string DbEntry::to_string() const {
    auto time_point = std::chrono::system_clock::from_time_t(seconds);
    auto time_str = mmotd::chrono::io::to_string(time_point, "%d-%h-%Y %I:%M%p %Z");
    return format(FMT_STRING("user: {}, device: {}, hostname: '{}', time: {}, ip: {}, type: {}"),
                  user,
                  device_name,
                  hostname,
                  time_str,
                  ip.to_string(),
                  DbEntry::entry_type_to_string(static_cast<int>(type)));
}

DbEntry DbEntry::from_utmpx(const utmpx &db) {
    if (db.ut_type != USER_PROCESS && db.ut_type != LOGIN_PROCESS) {
        return DbEntry{};
    }

    auto device_name = strlen(db.ut_line) > 0 ? string{db.ut_line} : string{};
    auto username = strlen(db.ut_user) > 0 ? string{db.ut_user} : string{};
    auto hostname = strlen(db.ut_host) > 0 ? string{db.ut_host} : string{};

    LOG_VERBOSE("utmpx id: {}", GetUtmpxId(db));
    auto entry = DbEntry{static_cast<ENTRY_TYPE>(db.ut_type),
                         device_name,
                         username,
                         hostname,
                         db.ut_tv.tv_sec,
                         boost::asio::ip::address{}};
    LOG_VERBOSE("parsed entry: {}", entry.to_string());
    return entry;
}

string DbEntry::entry_type_to_string(int ut_type) {
    static_assert(ENTRY_TYPE::None == static_cast<ENTRY_TYPE>(EMPTY));
    static_assert(ENTRY_TYPE::Login == static_cast<ENTRY_TYPE>(LOGIN_PROCESS));
    static_assert(ENTRY_TYPE::User == static_cast<ENTRY_TYPE>(USER_PROCESS));
    switch (ut_type) {
        case EMPTY:
            return "empty";
        case RUN_LVL:
            return "run lvl";
        case BOOT_TIME:
            return "boot time";
        case OLD_TIME:
            return "old time";
        case NEW_TIME:
            return "new time";
        case INIT_PROCESS:
            return "init process";
        case LOGIN_PROCESS:
            return "login process";
        case USER_PROCESS:
            return "user process";
        case DEAD_PROCESS:
            return "dead process";
        default:
            return "unknown";
    }
}

const DbEntries &detail::GetUserAccountEntries() {
    static const auto &entries = GetUserAccountEntriesImpl();
    return entries;
}

} // namespace mmotd::platform::user_account_database
#endif
