// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__linux__)
#include "common/include/chrono_io.h"
#include "common/include/logging.h"
#include "common/include/posix_error.h"
#include "lib/include/platform/user_accounting_database.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <boost/asio/ip/address.hpp>
#include <fmt/format.h>
#include <scope_guard.hpp>

#include <pwd.h>
#include <unistd.h>
#include <utmpx.h>

#include <utmp.h>

using boost::asio::ip::make_address;
using ip_address = boost::asio::ip::address;
namespace pe = mmotd::error::posix_error;
using fmt::format;
using namespace std;

namespace {

using namespace mmotd::platform::user;
using namespace mmotd::platform::user_account_database;

DbEntries GetUserAccountEntriesImpl() {
    // resets the database, so that the next getutxent() call will get the first entry
    setutxent();

    // read the next entry from the database; if the database was not yet open, it also opens it
    auto utmp_buf = utmp{};
    utmp *utmp_ptr = nullptr;
    auto retval = getutent_r(&utmp_buf, &utmp_ptr);
    if (retval != 0 || utmp_ptr == nullptr) {
        auto error_str = retval != 0 ? pe::to_string(retval) : pe::to_string();
        LOG_ERROR("attempting to read first value in user accounting database (utmp) and failed, {}", error_str);
        return DbEntries{};
    }

    // auto close the database however we leave this function
    auto endutxent_closer = sg::make_scope_guard([]() noexcept { endutxent(); });

    auto user_account_entries = DbEntries{};
    size_t i = 0;
    while (utmp_ptr != nullptr) {
        auto ut_type_str = DbEntry::entry_type_to_string(utmp_ptr->ut_type);
        LOG_VERBOSE("iteration #{}, type: {}, utmpx *: {}", ++i, ut_type_str, fmt::ptr(utmp_ptr));

        auto entry = DbEntry::from_utmp(*utmp_ptr);
        if (!entry.empty()) {
            user_account_entries.push_back(entry);
            LOG_VERBOSE("adding user account entry #{}", user_account_entries.size());
        }

        // read the next entry from the database
        memset(&utmp_buf, 0, sizeof(utmp));
        utmp_ptr = nullptr;
        retval = getutent_r(&utmp_buf, &utmp_ptr);
        if (retval != 0 || utmp_ptr == nullptr) {
            auto error_value = errno;
            auto error_str = pe::to_string(error_value);
            if (error_value == ENOENT) {
                LOG_VERBOSE("found the end of user accounting database (utmp), {}", error_str);
            } else {
                LOG_WARNING("attempting to read next value in user accounting database (utmp) and failed, {}",
                            error_str);
            }
            utmp_ptr = nullptr;
        }
    }
    LOG_VERBOSE("returning {} user account entries", user_account_entries.size());
    return user_account_entries;
}

UserInformation GetUserInformationImpl() {
    auto bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1) {
        bufsize = 16 * 1024;
    }

    auto buf = vector<char>(static_cast<size_t>(bufsize));
    auto pwd = passwd{};
    passwd *pwd_ptr = nullptr;
    auto user_id = geteuid();
    auto retval = getpwuid_r(user_id, &pwd, buf.data(), buf.size(), &pwd_ptr);
    if (pwd_ptr == nullptr && retval == 0) {
        LOG_ERROR("getpwnam_r for user id {} did not return a pwd structure or an error code", user_id);
        return UserInformation{};
    } else if (pwd_ptr == nullptr && retval != 0) {
        auto error_str = pe::to_string(retval);
        LOG_ERROR("getpwnam_r for user id {} failed, {}", user_id, error_str);
        return UserInformation{};
    }
    return UserInformation::from_passwd(pwd);
}

} // namespace

namespace mmotd::platform::user_account_database {

string DbEntry::to_string() const {
    auto time_point = std::chrono::system_clock::from_time_t(seconds);
    auto time_str = mmotd::chrono::io::to_string(time_point, "%d-%h-%Y %I:%M%p %Z");
    return format(FMT_STRING("user: {}, device: {}, hostname: {}, time: {}, ip: {}, type: {}"),
                  user,
                  device_name,
                  hostname,
                  time_str,
                  ip.to_string(),
                  DbEntry::entry_type_to_string(static_cast<int>(type)));
}

DbEntry DbEntry::from_utmp(const utmp &db) {
    if (db.ut_type != USER_PROCESS && db.ut_type != LOGIN_PROCESS) {
        return DbEntry{};
    }

    auto device_name = strlen(db.ut_line) > 0 ? string{db.ut_line} : string{};
    auto username = strlen(db.ut_user) > 0 ? string{db.ut_user} : string{};
    auto hostname = strlen(db.ut_host) > 0 ? string{db.ut_host} : string{};

    auto buffer = vector<char>(sizeof(db.ut_addr_v6), 0);
    memcpy(data(buffer), db.ut_addr_v6, size(buffer));
    auto ec = boost::system::error_code{};
    auto ip = make_address(data(buffer), ec);
    if (ec) {
        ip = ip_address{};
    }
    auto entry = DbEntry{static_cast<ENTRY_TYPE>(db.ut_type), device_name, username, hostname, db.ut_tv.tv_sec, ip};
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
    static const auto &user_account_entries = GetUserAccountEntriesImpl();
    return user_account_entries;
}

} // namespace mmotd::platform::user_account_database

namespace mmotd::platform::user {

UserInformation UserInformation::from_passwd(const passwd &pw) {
    auto username = strlen(pw.pw_name) > 0 ? string{pw.pw_name} : string{};
    auto user_id = pw.pw_uid;
    auto group_id = pw.pw_gid;
    auto full_name = strlen(pw.pw_gecos) > 0 ? string{pw.pw_gecos} : string{};
    auto home_directory = strlen(pw.pw_dir) > 0 ? string{pw.pw_dir} : string{};
    auto shell = strlen(pw.pw_shell) > 0 ? string{pw.pw_shell} : string{};
    return UserInformation{username, user_id, group_id, full_name, home_directory, shell};
}

const UserInformation &GetUserInformation() {
    static const auto &user_information = GetUserInformationImpl();
    return user_information;
}

} // namespace mmotd::platform::user
#endif
