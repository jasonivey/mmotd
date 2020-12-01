// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/chrono-io.h"
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

mmotd::ComputerValues ParseLastLogin(const struct lastlogx *login_record, pid_t user_id, const string &user_name) {
    assert(login_record != nullptr);
    time_t seconds = login_record->ll_tv.tv_sec;
    int32_t useconds = login_record->ll_tv.tv_usec;
    auto tty_name = string{strlen(login_record->ll_line) > 0 ? login_record->ll_line : ""};
    auto host_name = string{strlen(login_record->ll_host) > 0 ? login_record->ll_host : ""};

    auto computer_values = mmotd::ComputerValues{};
    auto local_time = fmt::localtime(seconds);
    auto local_time_str = format("{:%d-%h-%Y %I:%M:%S%p %Z}", local_time);
    PLOG_INFO << format("login time: {}", local_time_str);
    computer_values.emplace_back(make_tuple("last login", format("seconds: {}", seconds)));
    computer_values.emplace_back(make_tuple("last login", format("useconds: {}", useconds)));
    computer_values.emplace_back(make_tuple("last login", format("time: {}", local_time_str)));
    computer_values.emplace_back(make_tuple("last login", format("terminal name: {}", tty_name)));
    computer_values.emplace_back(make_tuple("last login", format("host name: {}", host_name)));
    computer_values.emplace_back(make_tuple("last login", format("user id: {}", user_id)));
    computer_values.emplace_back(make_tuple("last login", format("user name: {}", user_name)));
    return computer_values;
}

} // namespace detail

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
