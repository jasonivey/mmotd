#pragma once
#include <ctime>
#include <string>

struct LastLoginData {
    pid_t       user_id = 0;
    std::string user_name;
    time_t      seconds = 0;
    int32_t     useconds = 0;
    std::string tty_name;
    std::string host_name;
};

struct UserLoginLogoutTransaction {
    std::string user;
    std::string id;
    std::string tty;
    int32_t     pid = 0;
    int16_t     type = 0;
    time_t      seconds = 0;
    int32_t     useconds = 0;
    std::string host;
};

class LastLog {
public:
    LastLog() = default;
    ~LastLog() = default;

    bool GetNextRecord(UserLoginLogoutTransaction &transaction);
    bool GetLastLoginRecord(LastLoginData &data);

private:

};
