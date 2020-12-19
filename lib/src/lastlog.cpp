// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/chrono_io.h"
#include "common/include/posix_error.h"
#include "lib/include/computer_information.h"
#include "lib/include/lastlog.h"

#include <array>
#include <cassert>
#include <chrono>
#include <ctime>
#include <string>

#include <fmt/format.h>
#include <plog/Log.h>
#include <scope_guard.hpp>

#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <utmpx.h>

using fmt::format;
using namespace std;

bool gLinkLastLog = false;

namespace mmotd {

static const bool last_log_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::LastLog>(); });

namespace detail {

mmotd::ComputerValues ParseLastLogin(const struct lastlogx &login_record, pid_t /*user_id*/, const string &user_name) {
    time_t seconds = login_record.ll_tv.tv_sec;
    auto terminal = string{strlen(login_record.ll_line) > 0 ? login_record.ll_line : ""};
    auto host_name = string{strlen(login_record.ll_host) > 0 ? login_record.ll_host : ""};

    auto last_log_str = format("{} logged into {}", user_name, terminal);
    if (!host_name.empty()) {
        last_log_str += format(" from {}", host_name);
    }
    auto time_point = std::chrono::system_clock::from_time_t(seconds);
    auto login_str = format("Log in: {}", mmotd::chrono::io::to_string(time_point, "{:%d-%h-%Y %I:%M:%S%p %Z}"));
    auto logout_str = string{"Log out: still logged in"};

    PLOG_INFO << format("last login: {}", last_log_str);
    PLOG_INFO << format("last login: {}", login_str);
    PLOG_INFO << format("last login: {}", logout_str);

    auto computer_values = mmotd::ComputerValues{};
    computer_values.emplace_back(make_tuple("last login", last_log_str));
    computer_values.emplace_back(make_tuple("last login", login_str));
    computer_values.emplace_back(make_tuple("last login", logout_str));
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
    return !last_login_details_.empty() ? make_optional(last_login_details_) : nullopt;
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
        auto error_str = mmotd::error::posix_error::to_string();
        PLOG_ERROR << format("getpwuid_r failed uid: {}, details: {}", uid, error_str);
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
    last_login_details_ = detail::ParseLastLogin(*login_record, user_id, user_name);
    return true;
}

} // namespace mmotd
