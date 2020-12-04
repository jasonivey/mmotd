// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/users_logged_in.h"

#include <fmt/format.h>
#include <plog/Log.h>
#include <scope_guard.hpp>
#include <unordered_map>

#include <utmpx.h>

using fmt::format;
using namespace std;

bool gLinkUsersLoggedIn = false;

namespace mmotd {

static const bool users_logged_in_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::UsersLoggedIn>(); });

bool UsersLoggedIn::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        return GetUsersLoggedIn();
    }
    return has_queried;
}

optional<mmotd::ComputerValues> UsersLoggedIn::GetInformation() const {
    return !details_.empty() ? make_optional(details_) : nullopt;
}

namespace detail {

string to_string_ut_type(int ut_type) {
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

struct UserSession {
    string name;
    vector<string> ttys;
    string hostname;
};

vector<UserSession> GetUserSessions() {
    // resets the database, so that the next getutxent() call will get the first entry
    setutxent();

    // read the next entry from the database; if the database was not yet open, it also opens it
    struct utmpx *utx = getutxent();
    if (utx == nullptr) {
        PLOG_ERROR << "attempting to get logged in users failed, getutxent returned nullptr";
        return vector<UserSession>{};
    }

    // auto close the database however we leave this function
    auto endutxent_closer = sg::make_scope_guard([]() { endutxent(); });

    auto user_sessions = vector<detail::UserSession>{};
    size_t i = 0;
    while (utx != nullptr) {
        auto ut_type_str = detail::to_string_ut_type(utx->ut_type);
        PLOG_VERBOSE << format("{} iterating... type: {}, utmpx *: {}", ++i, ut_type_str, fmt::ptr(utx));
        if (utx->ut_type == USER_PROCESS) {
            auto username = strlen(utx->ut_user) > 0 ? string{utx->ut_user} : string{};
            auto tty = strlen(utx->ut_line) > 0 ? string{utx->ut_line} : string{};
            auto hostname = strlen(utx->ut_host) > 0 ? string{utx->ut_host} : string{};
            if (username.empty() || tty.empty()) {
                PLOG_WARNING << format("attempting to get users failed user ({}), tty ({}) or host name ({}) is blank",
                                       username,
                                       tty,
                                       hostname);
            } else {
                user_sessions.push_back(detail::UserSession{username, vector<string>{tty}, hostname});
                PLOG_DEBUG << format("adding session {}: {}", user_sessions.size(), username);
            }
        }
        // read the next entry from the database
        utx = getutxent();
    }

    return user_sessions;
}

unordered_map<string, detail::UserSession> FilterUserSessions(const vector<detail::UserSession> &input_user_sessions) {
    auto user_sessions = unordered_map<string, detail::UserSession>{};
    for_each(begin(input_user_sessions), end(input_user_sessions), [&user_sessions](const auto &user_session) {
        auto count = user_sessions.count(user_session.name);
        if (count > 0) {
            assert(user_session.ttys.size() == 1);
            user_sessions[user_session.name].ttys.push_back(user_session.ttys.front());
            if (user_sessions[user_session.name].hostname.empty() && !user_session.hostname.empty()) {
                user_sessions[user_session.name].hostname = user_session.hostname;
            }
        } else {
            user_sessions[user_session.name] = user_session;
        }
    });
    return user_sessions;
}

mmotd::ComputerValues CreateUserSessions(const unordered_map<string, detail::UserSession> &user_sessions) {
    auto values = mmotd::ComputerValues{};
    for (const auto &user_session_pair : user_sessions) {
        const auto &user_session = user_session_pair.second;
        auto session_str = string{};
        session_str += format("{} logged in {} time{}",
                              user_session.name,
                              user_session.ttys.size(),
                              user_session.ttys.size() > 1 ? "s" : "");
        if (!user_session.hostname.empty()) {
            session_str += format(" from {}", user_session.hostname);
        }
        auto ttys_str = string{};
        for (const auto &tty : user_session.ttys) {
            ttys_str += format("{}{}", ttys_str.empty() ? "" : ", ", tty);
        }
        //session_str += format(", tty's: {}", ttys_str);
        values.push_back(make_tuple("user session", session_str));
    }
    return values;
}

} // namespace detail

bool UsersLoggedIn::GetUsersLoggedIn() {
    details_.clear();
    auto utmpx_user_sessions = detail::GetUserSessions();
    auto user_sessions = detail::FilterUserSessions(utmpx_user_sessions);
    details_ = detail::CreateUserSessions(user_sessions);
    return !details_.empty();
}

} // namespace mmotd
