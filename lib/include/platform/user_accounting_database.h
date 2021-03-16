// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <algorithm>
#include <climits>
#include <cstdint>
#include <ctime>
#include <iterator>
#include <string>
#include <vector>

#include <boost/asio/ip/address.hpp>

struct passwd;
struct utmp;
struct utmpx;

namespace mmotd::platform::user_account_database {

// User Account Entry Type
enum class ENTRY_TYPE : int { None = 0, Login = 6, User = 7 };

struct DbEntry {
    ENTRY_TYPE type = ENTRY_TYPE::None;
    std::string device_name; // tty name or console
    std::string user;
    std::string hostname;
    std::time_t seconds = 0; //time_t seconds = record.ll_tv.tv_sec;
    boost::asio::ip::address ip;

    std::string to_string() const;
    static std::string entry_type_to_string(int);
    bool empty() const { return type == ENTRY_TYPE::None; }
    bool is_login() const { return type == ENTRY_TYPE::Login; }
    bool is_user() const { return type == ENTRY_TYPE::User; }

    static DbEntry from_utmp(const utmp &db);
    static DbEntry from_utmpx(const utmpx &db);
};

using DbEntries = std::vector<DbEntry>;

namespace detail {

const DbEntries &GetUserAccountEntries();

}

template<ENTRY_TYPE type>
inline DbEntries GetDbEntries();

template<>
inline DbEntries GetDbEntries<ENTRY_TYPE::Login>() {
    auto entries = detail::GetUserAccountEntries();
    auto i = std::remove_if(std::begin(entries), std::end(entries), [](const auto &entry) {
        return entry.type != ENTRY_TYPE::Login;
    });
    entries.erase(i, end(entries));
    return entries;
}

template<>
inline DbEntries GetDbEntries<ENTRY_TYPE::User>() {
    auto entries = detail::GetUserAccountEntries();
    auto i = std::remove_if(std::begin(entries), std::end(entries), [](const auto &entry) {
        return entry.type != ENTRY_TYPE::User;
    });
    entries.erase(i, end(entries));
    return entries;
}

} // namespace mmotd::platform::user_account_database

namespace mmotd::platform::user {

struct UserInformation {
    std::string username;
    uid_t user_id = UINT_MAX;
    gid_t group_id = UINT_MAX;
    std::string full_name;
    std::string home_directory;
    std::string shell;

    bool empty() const { return user_id == UINT_MAX && username.empty(); }

    static UserInformation from_passwd(const passwd &pw);
};

const UserInformation &GetUserInformation();

} // namespace mmotd::platform::user
