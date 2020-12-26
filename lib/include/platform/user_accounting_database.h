// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <algorithm>
#include <climits>
#include <cstdint>
#include <ctime>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include <boost/asio/ip/address.hpp>

struct passwd;
struct utmp;
struct utmpx;

namespace mmotd::platform {

// User Account Entry Type
enum class UAE_TYPE : int { None = 0, Login = 6, User = 7 };

struct UserAccountEntry {
    UAE_TYPE type = UAE_TYPE::None;
    // also tty
    std::string device_name;
    std::string user;
    std::string hostname;
    std::time_t seconds = 0; //time_t seconds = record.ll_tv.tv_sec;
    boost::asio::ip::address ip;

    std::string to_string() const;
    static std::string to_type_string(int);
    std::string type_str() const;
    bool empty() const { return type == UAE_TYPE::None; }
    bool is_login() const { return type == UAE_TYPE::Login; }
    bool is_user() const { return type == UAE_TYPE::User; }

    static UserAccountEntry from_utmp(const utmp &db);
    static UserAccountEntry from_utmpx(const utmpx &db);
};

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

using UserAccountEntries = std::vector<UserAccountEntry>;

const UserAccountEntries &GetUserAccountEntries();

inline UserAccountEntries GetUserAccountEntries(UAE_TYPE type) {
    auto entries = GetUserAccountEntries();
    auto i = std::remove_if(std::begin(entries), std::end(entries), [type](const auto &entry) {
        return entry.type != type;
    });
    entries.erase(i, end(entries));
    return entries;
}

const UserInformation &GetUserInformation();

} // namespace mmotd::platform
