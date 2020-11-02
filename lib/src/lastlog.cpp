#include "lib/include/lastlog.h"

#include <cassert>
#include <utmpx.h>
#include <ctime>
#include <string>
#include <iostream>
#include <unistd.h>
#include <pwd.h>
#include <string.h>

/*
struct utmpx {
	char ut_user[_UTX_USERSIZE];	// login namegc
	char ut_id[_UTX_IDSIZE];	// idgc
	char ut_line[_UTX_LINESIZE];	// tty namegc
	pid_t ut_pid;			// process id creating the entrygc
	short ut_type;			// type of this entrygc
	struct timeval ut_tv;		// time entry was createdgc
	char ut_host[_UTX_HOSTSIZE];	// host namegc
	__uint32_t ut_pad[16];		// reserved for future usegc
};
*/

using namespace std;

static UserLoginLogoutTransaction CreateTransaction(const struct utmpx* utmpx_ptr) {
    assert(utmpx_ptr != nullptr);
    auto user = string{strlen(utmpx_ptr->ut_user) > 0 ? utmpx_ptr->ut_user : ""};
    auto id = string{strlen(utmpx_ptr->ut_id) > 0 ? utmpx_ptr->ut_id : ""};
    auto tty = string{strlen(utmpx_ptr->ut_line) > 0 ? utmpx_ptr->ut_line : ""};
    int32_t pid = utmpx_ptr->ut_pid;
    int16_t type = utmpx_ptr->ut_type;
    long seconds = utmpx_ptr->ut_tv.tv_sec;
    int32_t useconds = utmpx_ptr->ut_tv.tv_usec;
    auto host = string{strlen(utmpx_ptr->ut_host) > 0 ? utmpx_ptr->ut_host : ""};
    return UserLoginLogoutTransaction{user, id, tty, pid, type, seconds, useconds, host};
}

static const LastLoginData CreateLastLogin(const struct lastlogx *login_record, pid_t user_id, const string &user_name) {
    assert(login_record != nullptr);
    time_t seconds = login_record->ll_tv.tv_sec;
    int32_t useconds = login_record->ll_tv.tv_usec;
    auto tty_name = string{strlen(login_record->ll_line) > 0 ? login_record->ll_line : ""};
    auto host_name = string{strlen(login_record->ll_host) > 0 ? login_record->ll_host : ""};
    return LastLoginData{user_id, user_name, seconds, useconds, tty_name, host_name};
}

ostream &operator<<(ostream &out, const UserLoginLogoutTransaction &transaction) {
    switch (transaction.type) {
        case EMPTY:
            out << "type      : EMPTY\n";
            break;
        case RUN_LVL:
            out << "type      : RUN_LVL\n";
            break;
        case BOOT_TIME:
            out << "type      : BOOT_TIME\n";
            break;
        case OLD_TIME:
            out << "type      : OLD_TIME\n";
            break;
        case NEW_TIME:
            out << "type      : NEW_TIME\n";
            break;
        case INIT_PROCESS:
            out << "type      : INIT_PROCESS\n";
            break;
        case LOGIN_PROCESS:
            out << "type      : LOGIN_PROCESS\n";
            break;
        case USER_PROCESS:
            out << "type      : USER_PROCESS\n";
            break;
        case DEAD_PROCESS:
            out << "type      : DEAD_PROCESS\n";
            break;
        case ACCOUNTING:
            out << "type      : ACCOUNTING\n";
            break;
        case SIGNATURE:
            out << "type      : SIGNATURE\n";
            break;
        case SHUTDOWN_TIME:
            out << "type      : SHUTDOWN_TIME\n";
            break;
        default:
            out << "type      : unknown\n";
    }
    out << "  user    : " << transaction.user << "\n";
    out << "  id      : " << transaction.id << "\n";
    out << "  tty     : " << transaction.tty << "\n";
    out << "  host    : " << transaction.host << "\n";
    out << "  pid     : " << transaction.pid << "\n";
    out << "  seconds : " << transaction.seconds << "\n";
    out << "  useconds: " << transaction.useconds << "\n";
    return out;
}

ostream &operator<<(ostream &out, const LastLoginData &record) {
    out << "user id   : " << record.user_id << "\n";
    out << "user name : " << record.user_name << "\n";
    out << "seconds   : " << record.seconds << "\n";
    out << "useconds  : " << record.useconds << "\n";
    out << "tty       : " << record.tty_name << "\n";
    out << "host      : " << record.host_name << "\n";
    return out;
}

static void DbgDumpUserLoginLogoutTransaction(const UserLoginLogoutTransaction &transaction)  {
    cout << transaction;
}


static void DbgDumpLastLoginRecord(const LastLoginData &record) {
    cout << record;
}

bool LastLog::GetNextRecord(UserLoginLogoutTransaction &) {
    //char * ctime(const time_t *clock);
    struct utmpx *utmpx_ptr = getutxent_wtmp();
    if (utmpx_ptr == nullptr) {
        cerr << "ERROR: getutxent_wtmp did not return a record\n";
        return false;
    }

    auto count = size_t{0};
    while (utmpx_ptr != nullptr) {
        ++count;
        auto transaction = CreateTransaction(utmpx_ptr);
        DbgDumpUserLoginLogoutTransaction(transaction);
        utmpx_ptr = getutxent_wtmp();
    }
    cout << "Total Records: " << count << endl;
    endutxent();
    return true;
}

bool LastLog::GetLastLoginRecord(LastLoginData &) {
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if (pw == nullptr) {
        cerr << "ERROR: getpwuid failed to return a user name\n";
        return false;
    }
    auto user_name = std::string{pw->pw_name};
    struct lastlogx *login_record = getlastlogx(uid, nullptr);
    if (login_record == nullptr) {
        cerr << "ERROR: getlastlogx did not return a record\n";
        return false;
    }
    const auto lastlogin_record = CreateLastLogin(login_record, uid, user_name);
    DbgDumpLastLoginRecord(lastlogin_record);
    return true;
}

