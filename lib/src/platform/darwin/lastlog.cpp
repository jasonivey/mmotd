// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/chrono_io.h"
#include "lib/include/platform/lastlog.h"
#include "lib/include/platform/user_accounting_database.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <tuple>
#include <vector>

#include <fmt/format.h>
#include <plog/Log.h>
using mmotd::chrono::io::to_string;

using fmt::format;
using namespace std;

namespace mmotd::platform {

LastLoginDetails GetLastLogDetails() {
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
        return LastLoginDetails{};
    }

    auto i =
        min_element(begin(entries), end(entries), [](const auto &a, const auto &b) { return a.seconds < b.seconds; });

    if (i == end(entries)) {
        PLOG_ERROR << "last login: unable to find a user process to use";
        return LastLoginDetails{};
    }

    const auto &entry = *i;
    PLOG_VERBOSE << format("last log: found {}", entry.to_string());

    auto summary = format("{} logged into {}", entry.user, entry.device_name);
    if (!entry.hostname.empty()) {
        summary += format(" from {}", entry.hostname);
    }

    auto log_in_time = std::chrono::system_clock::from_time_t(entry.seconds);
    auto details = LastLoginDetails{summary, log_in_time, std::chrono::system_clock::time_point{}};

    PLOG_VERBOSE << format("last login: {}", details.summary);
    PLOG_VERBOSE << format("last log in: {}", to_string(details.log_in, "%d-%h-%Y %I:%M:%S%p %Z"));
    PLOG_VERBOSE << "last log out: still logged in";

    return details;
}

} // namespace mmotd::platform
