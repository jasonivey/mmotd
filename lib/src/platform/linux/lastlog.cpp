// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/chrono_io.h"
#include "lib/include/platform/lastlog.h"
#include "lib/include/platform/user_accounting_database.h"

#include <algorithm>
#include <chrono>
#include <iterator>
#include <string>
#include <tuple>
#include <vector>

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

namespace mmotd::platform {

LastLogDetails GetLastLogDetails() {
    using namespace mmotd::platform::user_account_database;
    using namespace mmotd::platform::user;

    auto entries = GetDbEntries<ENTRY_TYPE::User>();
    auto user_info = GetUserInformation();
    PLOG_VERBOSE << format("last login: entries size: {}, user info: {}",
                           entries.size(),
                           user_info.empty() ? "empty" : user_info.username);

    if (entries.empty() || user_info.empty()) {
        PLOG_ERROR << format("last login: entries size: {}, user info: {}",
                             entries.size(),
                             user_info.empty() ? "empty" : "valid");
        return LastLogDetails{};
    }

    auto i =
        min_element(begin(entries), end(entries), [](const auto &a, const auto &b) { return a.seconds < b.seconds; });

    if (i == end(entries)) {
        PLOG_ERROR << "last login: unable to find a user process to use";
        return LastLogDetails{};
    }

    const auto &entry = *i;
    PLOG_VERBOSE << format("last log: found {}", entry.to_string());

    auto last_log_str = format("{} logged into {}", entry.user, entry.device_name);
    if (!entry.hostname.empty()) {
        last_log_str += format(" from {}", entry.hostname);
    }

    // last login:
    auto details = LastLogDetails{};
    details.emplace_back(make_tuple("last log", last_log_str));
    PLOG_VERBOSE << format("last login: {}", last_log_str);
    // log in:
    auto time_point = std::chrono::system_clock::from_time_t(entry.seconds);
    auto login_str = mmotd::chrono::io::to_string(time_point, "{:%d-%h-%Y %I:%M:%S%p %Z}");
    details.emplace_back(make_tuple("log in", login_str));
    PLOG_VERBOSE << format("last login: {}", login_str);
    // log out:
    auto logout_str = string{"still logged in"};
    details.emplace_back(make_tuple("log out", logout_str));
    PLOG_VERBOSE << format("last login: {}", logout_str);

    return details;
}

} // namespace mmotd::platform
