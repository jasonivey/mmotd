// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/chrono_io.h"
#include "common/include/posix_error.h"
#include "lib/include/platform/user_accounting_database.h"

#include <cstring>
#include <string>
#include <vector>

#include <boost/asio/ip/address.hpp>
#include <fmt/format.h>
#include <plog/Log.h>
#include <scope_guard.hpp>

#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utmpx.h>

#include <utmp.h>

using boost::asio::ip::make_address;
using ip_address = boost::asio::ip::address;
namespace errorp = mmotd::error::posix_error;
using fmt::format;
using namespace std;

namespace {

mmotd::platform::UserAccountEntries GetUserAccountEntriesImpl() {
    // resets the database, so that the next getutxent() call will get the first entry
    setutxent();

    // read the next entry from the database; if the database was not yet open, it also opens it
    auto utmp_buf = utmp{};
    utmp *utmp_ptr = nullptr;
    auto retval = getutent_r(&utmp_buf, &utmp_ptr);
    if (retval != 0 || utmp_ptr == nullptr) {
        auto error_str = retval != 0 ? errorp::to_string(retval) : errorp::to_string();
        PLOG_ERROR << format("attempting to read first value in user accounting database (utmp) and failed, {}",
                             error_str);
        return mmotd::platform::UserAccountEntries{};
    }

    // auto close the database however we leave this function
    auto endutxent_closer = sg::make_scope_guard([]() { endutxent(); });

    auto user_account_enteries = mmotd::platform::UserAccountEntries{};
    size_t i = 0;
    while (utmp_ptr != nullptr) {
        auto ut_type_str = mmotd::platform::UserAccountEntry::to_type_string(utmp_ptr->ut_type);
        PLOG_VERBOSE << format("iteration #{}, type: {}, utmpx *: {}", ++i, ut_type_str, fmt::ptr(utmp_ptr));

        auto entry = mmotd::platform::UserAccountEntry::from_utmp(*utmp_ptr);
        if (!entry.empty()) {
            user_account_enteries.push_back(entry);
            PLOG_VERBOSE << format("adding user account entry #{}", user_account_enteries.size());
        }

        // read the next entry from the database
        memset(&utmp_buf, 0, sizeof(utmp));
        utmp_ptr = nullptr;
        retval = getutent_r(&utmp_buf, &utmp_ptr);
        if (retval != 0 || utmp_ptr == nullptr) {
            auto error_value = errno;
            auto error_str = errorp::to_string(error_value);
            if (error_value == ENOENT) {
                PLOG_VERBOSE << format("found the end of user accounting database (utmp), {}", error_str);
            } else {
                PLOG_ERROR << format("attempting to read next value in user accounting database (utmp) and failed, {}",
                                     error_str);
            }
            utmp_ptr = nullptr;
        }
    }
    PLOG_VERBOSE << format("returning {} user account entries", user_account_enteries.size());
    return user_account_enteries;
}

mmotd::platform::UserInformation GetUserInformationImpl() {
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
        PLOG_ERROR << format("getpwnam_r for user id {} did not return a pwd structure or an error code", user_id);
        return mmotd::platform::UserInformation{};
    } else if (pwd_ptr == nullptr && retval != 0) {
        auto error_str = errorp::to_string(retval);
        PLOG_ERROR << format("getpwnam_r for user id {} failed, {}", user_id, error_str);
        return mmotd::platform::UserInformation{};
    }
    return mmotd::platform::UserInformation::from_passwd(pwd);
}

} // namespace

namespace mmotd::platform {

string UserAccountEntry::to_type_string(int ut_type) {
    static_assert(UAE_TYPE::None == static_cast<UAE_TYPE>(EMPTY));
    static_assert(UAE_TYPE::Login == static_cast<UAE_TYPE>(LOGIN_PROCESS));
    static_assert(UAE_TYPE::User == static_cast<UAE_TYPE>(USER_PROCESS));
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

const UserAccountEntries &GetUserAccountEntries() {
    static auto user_account_enteries = GetUserAccountEntriesImpl();
    return user_account_enteries;
}

const UserInformation &GetUserInformation() {
    static auto user_information = GetUserInformationImpl();
    return user_information;
}

string UserAccountEntry::to_string() const {
    auto time_point = std::chrono::system_clock::from_time_t(seconds);
    auto time_str = mmotd::chrono::io::to_string(time_point, "{:%d-%h-%Y %I:%M:%S%p %Z}");
    return format("user: {}, device: {}, hostname: {}, time: {}, ip: {}, type: {}",
                  user,
                  device_name,
                  hostname,
                  time_str,
                  ip.to_string(),
                  UserAccountEntry::to_type_string(static_cast<int>(type)));
}

UserAccountEntry UserAccountEntry::from_utmp(const utmp &db) {
    if (db.ut_type != USER_PROCESS && db.ut_type != LOGIN_PROCESS) {
        return UserAccountEntry{};
    }

    auto device_name = strlen(db.ut_line) > 0 ? string{db.ut_line} : string{};
    auto username = strlen(db.ut_user) > 0 ? string{db.ut_user} : string{};
    auto hostname = strlen(db.ut_host) > 0 ? string{db.ut_host} : string{};

    auto buffer = vector<char>{sizeof(int32_t) * 4, 0};
    memcpy(buffer.data(), db.ut_addr_v6, buffer.size());
    auto ec = boost::system::error_code{};
    auto ip = make_address(buffer.data(), ec);
    if (ec) {
        ip = ip_address{};
    }
    auto entry =
        UserAccountEntry{static_cast<UAE_TYPE>(db.ut_type), device_name, username, hostname, db.ut_tv.tv_sec, ip};
    PLOG_VERBOSE << format("parsed entry: {}", entry.to_string());
    return entry;
}

UserInformation UserInformation::from_passwd(const passwd &pw) {
    auto username = strlen(pw.pw_name) > 0 ? string{pw.pw_name} : string{};
    auto user_id = pw.pw_uid;
    auto group_id = pw.pw_gid;
    auto full_name = strlen(pw.pw_gecos) > 0 ? string{pw.pw_gecos} : string{};
    auto home_directory = strlen(pw.pw_dir) > 0 ? string{pw.pw_dir} : string{};
    auto shell = strlen(pw.pw_shell) > 0 ? string{pw.pw_shell} : string{};
    return UserInformation{username, user_id, group_id, full_name, home_directory, shell};
}

} // namespace mmotd::platform
