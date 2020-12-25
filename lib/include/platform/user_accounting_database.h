// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <ctime>
#include <climits>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <boost/asio/ip/address.hpp>

struct passwd;
struct utmp;

namespace mmotd::platform {

struct UserAccountEntry {
    int type = 0;
    // also tty
    std::string device_name;
    std::string user;
    std::string hostname;
    std::time_t seconds = 0; //time_t seconds = record.ll_tv.tv_sec;
    boost::asio::ip::address ip;

    std::string to_string() const;
    static std::string to_type_string(int);
    std::string type_str() const;
    bool empty() const { return type == 0; }
    bool is_login() const { return type == 6; }
    bool is_user() const { return type == 7; }

    static UserAccountEntry from_utmp(const utmp &db);
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

const UserInformation &GetUserInformation();

} // namespace mmotd::platform

