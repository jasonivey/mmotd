// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/lastlog.h"

#include <array>
#include <cassert>
#include <chrono>
#include <ctime>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <plog/Log.h>
#include <pwd.h>
#include <scope_guard.hpp>
#include <string.h>
#include <string>
#include <unistd.h>
#include <utmpx.h>

using fmt::format;
using namespace std;

bool gLinkLastLog = false;

namespace mmotd {

static const bool last_log_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::LastLog>(); });

namespace detail {

// struct utmpx {
//     char ut_user[_UTX_USERSIZE];	// login namegc
//     char ut_id[_UTX_IDSIZE];	// idgc
//     char ut_line[_UTX_LINESIZE];	// tty namegc
//     pid_t ut_pid;			// process id creating the entrygc
//     short ut_type;			// type of this entrygc
//     struct timeval ut_tv;		// time entry was createdgc
//     char ut_host[_UTX_HOSTSIZE];	// host namegc
//     __uint32_t ut_pad[16];		// reserved for future usegc
// };

// struct LastLoginData {
//     pid_t user_id = 0;
//     std::string user_name;
//     time_t seconds = 0;
//     int32_t useconds = 0;
//     std::string tty_name;
//     std::string host_name;
// };

// struct UserLoginLogoutTransaction {
//     std::string user;
//     std::string id;
//     std::string tty;
//     int32_t pid = 0;
//     int16_t type = 0;
//     time_t seconds = 0;
//     int32_t useconds = 0;
//     std::string host;
// };

// static void DbgDumpUserLoginLogoutTransaction(const UserLoginLogoutTransaction &transaction) {
//     cout << transaction;
// }

// static void DbgDumpLastLoginRecord(const LastLoginData &record) {
//     cout << record;
// }

// UserLoginLogoutTransaction CreateTransaction(const struct utmpx *utmpx_ptr) {
//     assert(utmpx_ptr != nullptr);
//     auto user = string{strlen(utmpx_ptr->ut_user) > 0 ? utmpx_ptr->ut_user : ""};
//     auto id = string{strlen(utmpx_ptr->ut_id) > 0 ? utmpx_ptr->ut_id : ""};
//     auto tty = string{strlen(utmpx_ptr->ut_line) > 0 ? utmpx_ptr->ut_line : ""};
//     int32_t pid = utmpx_ptr->ut_pid;
//     int16_t type = utmpx_ptr->ut_type;
//     long seconds = utmpx_ptr->ut_tv.tv_sec;
//     int32_t useconds = utmpx_ptr->ut_tv.tv_usec;
//     auto host = string{strlen(utmpx_ptr->ut_host) > 0 ? utmpx_ptr->ut_host : ""};
//     return UserLoginLogoutTransaction{user, id, tty, pid, type, seconds, useconds, host};
// }

using time_point = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>;

time_point shift_epoch(std::chrono::seconds seconds, std::chrono::microseconds microseconds) {
    auto st = std::chrono::system_clock::from_time_t(static_cast<time_t>(seconds.count()));
    st += microseconds;
    PLOG_ERROR << format("login time: {}", to_string(st));
}

mmotd::ComputerValues ParseLastLogin(const struct lastlogx *login_record, pid_t user_id, const string &user_name) {
    assert(login_record != nullptr);
    time_t seconds = login_record->ll_tv.tv_sec;
    int32_t useconds = login_record->ll_tv.tv_usec;
    auto tty_name = string{strlen(login_record->ll_line) > 0 ? login_record->ll_line : ""};
    auto host_name = string{strlen(login_record->ll_host) > 0 ? login_record->ll_host : ""};

    auto computer_values = mmotd::ComputerValues{};
    computer_values.emplace_back(make_tuple("last login", format("seconds: {}", seconds)));
    computer_values.emplace_back(make_tuple("last login", format("useconds: {}", useconds)));
    computer_values.emplace_back(make_tuple("last login", format("terminal name: {}", tty_name)));
    computer_values.emplace_back(make_tuple("last login", format("host name: {}", host_name)));
    computer_values.emplace_back(make_tuple("last login", format("user id: {}", user_id)));
    computer_values.emplace_back(make_tuple("last login", format("user name: {}", user_name)));
    return computer_values;
}

} // namespace detail

// ostream &operator<<(ostream &out, const UserLoginLogoutTransaction &transaction) {
//     switch (transaction.type) {
//     case EMPTY:
//         out << "type      : EMPTY\n";
//         break;
//     case RUN_LVL:
//         out << "type      : RUN_LVL\n";
//         break;
//     case BOOT_TIME:
//         out << "type      : BOOT_TIME\n";
//         break;
//     case OLD_TIME:
//         out << "type      : OLD_TIME\n";
//         break;
//     case NEW_TIME:
//         out << "type      : NEW_TIME\n";
//         break;
//     case INIT_PROCESS:
//         out << "type      : INIT_PROCESS\n";
//         break;
//     case LOGIN_PROCESS:
//         out << "type      : LOGIN_PROCESS\n";
//         break;
//     case USER_PROCESS:
//         out << "type      : USER_PROCESS\n";
//         break;
//     case DEAD_PROCESS:
//         out << "type      : DEAD_PROCESS\n";
//         break;
//     case ACCOUNTING:
//         out << "type      : ACCOUNTING\n";
//         break;
//     case SIGNATURE:
//         out << "type      : SIGNATURE\n";
//         break;
//     case SHUTDOWN_TIME:
//         out << "type      : SHUTDOWN_TIME\n";
//         break;
//     default:
//         out << "type      : unknown\n";
//     }
//     out << "  user    : " << transaction.user << "\n";
//     out << "  id      : " << transaction.id << "\n";
//     out << "  tty     : " << transaction.tty << "\n";
//     out << "  host    : " << transaction.host << "\n";
//     out << "  pid     : " << transaction.pid << "\n";
//     out << "  seconds : " << transaction.seconds << "\n";
//     out << "  useconds: " << transaction.useconds << "\n";
//     return out;
// }

// ostream &operator<<(ostream &out, const LastLoginData &record) {
//     out << "user id   : " << record.user_id << "\n";
//     out << "user name : " << record.user_name << "\n";
//     out << "seconds   : " << record.seconds << "\n";
//     out << "useconds  : " << record.useconds << "\n";
//     out << "tty       : " << record.tty_name << "\n";
//     out << "host      : " << record.host_name << "\n";
//     return out;
// }

bool LastLog::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        return GetLastLoginRecord();
    }
    return has_queried;
}

std::optional<mmotd::ComputerValues> LastLog::GetInformation() const {
    return last_login_details_;
}

#if 0
bool LastLog::GetNextRecord(UserLoginLogoutTransaction &) {
    // char * ctime(const time_t *clock);
    struct utmpx *utmpx_ptr = getutxent_wtmp();
    if (utmpx_ptr == nullptr) {
        PLOG_ERROR << "getutxent_wtmp did not return a record";
    }

    auto count = size_t{0};
    while (utmpx_ptr != nullptr) {
        ++count;
        auto transaction = detail::CreateTransaction(utmpx_ptr);
        //DbgDumpUserLoginLogoutTransaction(transaction);
        utmpx_ptr = getutxent_wtmp();
    }
    PLOG_INFO << format("total records in last log: {}", count);
    endutxent();
    return true;
}
#endif

optional<tuple<string, uint32_t>> LastLog::GetUsername() {
    uid_t uid = geteuid();
    auto suggested_buf_size = sysconf(_SC_GETPW_R_SIZE_MAX);
    size_t buf_size = suggested_buf_size == -1 ? (16 * 1024) : static_cast<size_t>(suggested_buf_size);
    char *buf = reinterpret_cast<char *>(malloc(buf_size));
    if (buf == NULL) {
        PLOG_ERROR << format("malloc failed to allocate {} bytes to get user name", buf_size);
        return nullopt;
    }
    auto buf_deleter = sg::make_scope_guard([&buf]() { free(buf); });
    memset(buf, 0, buf_size);
    struct passwd pwd;
    struct passwd *result = nullptr;
    int retval = getpwuid_r(uid, &pwd, buf, buf_size, &result);
    if (retval != 0) {
        auto error_buf = array<char, 256>{};
        if (strerror_r(retval, error_buf.data(), error_buf.size()) == 0) {
            auto error_str = string{error_buf.begin(), error_buf.end()};
            PLOG_ERROR << format("getpwuid_r failed uid: {}, details: {}", uid, error_str);
        } else {
            PLOG_ERROR << format("getpwuid_r failed uid: {}, details: {}", uid);
        }
        return nullopt;
    } else if (result == nullptr) {
        PLOG_ERROR << format("getpwuid_r failed to find a user with uid: {}", uid);
        return nullopt;
    } else if (result->pw_name == nullptr || strlen(result->pw_name) == 0) {
        PLOG_ERROR << format("getpwuid_r appeared to succeed with uid {} but there is no user name", uid);
        return nullopt;
    }
    auto user_name = string{result->pw_name, result->pw_name + strlen(result->pw_name)};
    PLOG_INFO << format("found user: {}, with uid: {}", user_name, uid);
    return make_optional(make_tuple(user_name, uid));
}

bool LastLog::GetLastLoginRecord() {
    auto user_info_wrapper = GetUsername();
    if (!user_info_wrapper) {
        PLOG_ERROR << "unable to find user id and name";
        return false;
    }
    auto [user_name, user_id] = *user_info_wrapper;
    struct lastlogx *login_record = getlastlogxbyname(user_name.c_str(), nullptr);
    if (login_record == nullptr) {
        PLOG_ERROR << format("getlastlogxbyname did not return a record for user: {}", user_name);
        return false;
    }
    auto login_record_deleter = sg::make_scope_guard([&login_record]() { free(login_record); });
    auto login_details = detail::ParseLastLogin(login_record, user_id, user_name);
    last_login_details_ = login_details;
    return true;
}

} // namespace mmotd
