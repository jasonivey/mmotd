// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/algorithm.h"
#include "common/include/chrono_io.h"
#include "common/include/iostream_error.h"
#include "common/include/logging.h"
#include "common/include/user_information.h"
#include "lib/include/computer_information.h"
#include "lib/include/general.h"

#include <chrono>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <effolkronium/random.hpp>

using effing_random = effolkronium::random_static;
using mmotd::algorithms::value_in_range, mmotd::algorithms::value_outside_range;
using namespace std;

bool gLinkGeneralGenerator = false;

static const bool general_factory_registered =
    mmotd::information::RegisterInformationProvider([]() { return make_unique<mmotd::information::General>(); });

namespace {
using HourType = std::chrono::hours::rep;

class Greeting {
public:
    Greeting() = default;

    Greeting(string_view greet_str, HourType begin_hour, HourType end_hour, initializer_list<string_view> emojis) :
        greeting_str_(greet_str), begin_hour_(begin_hour), end_hour_(end_hour), emojis_(emojis) {}

    auto begin() const { return begin_hour_; }
    auto end() const { return end_hour_; }
    string GetTimeOfDayGreeting() const { return string{greeting_str_}; }
    string GetRandomEmoji() const {
        auto emoji = effing_random::get(emojis_);
        return string{*emoji};
    }

private:
    string_view greeting_str_;
    HourType begin_hour_ = HourType{0};
    HourType end_hour_ = HourType{0};
    vector<string_view> emojis_;
};

using namespace std::string_view_literals;

class Greetings {
public:
    Greetings() = default;

    string GetTimeOfDayGreeting() const;
    string GetTimeOfDayEmoji() const;
    string GetLocalDateTime() const;

private:
    static constexpr const auto MORNING_EMOJIS =
        {"🌤"sv, "⛅"sv, "🌦️"sv, "🌤️"sv, "🌥️"sv, "🌈"sv, "🌦"sv};
    static constexpr const auto AFTERNOON_EMOJIS = {"🌎"sv, "🌎"sv, "🌍"sv, "🌏"sv, "⛱"sv, "🏖"sv};
    static constexpr const auto EVENING_EMOJIS =
        {"🌙"sv, "🌖"sv, "🌕"sv, "🌓"sv, "🌛"sv, "🌝"sv, "🌗"sv, "🌜"sv, "🌑"sv, "🌚"sv, "🌘"sv, "🌒"sv, "🌔"sv};
    static constexpr const auto NIGHT_EMOJIS = {"⭐"sv, "💫"sv, "🌟"sv, "☄"sv};

    // Morning from 5am - 12pm
    const Greeting MORNING_GREETING = Greeting{"Morning"sv, HourType{5}, HourType{12}, MORNING_EMOJIS};
    // Afternoon from 12pm - 6pm
    const Greeting AFTERNOON_GREETING = Greeting{"Afternoon"sv, HourType{12}, HourType{18}, AFTERNOON_EMOJIS};
    // Evening from 6pm - 11pm
    const Greeting EVENING_GREETING = Greeting{"Evening"sv, HourType{18}, HourType{23}, EVENING_EMOJIS};
    // Night from 11pm - 4am
    const Greeting NIGHT_GREETING = Greeting{"Night"sv, HourType{5}, HourType{23}, NIGHT_EMOJIS};

    optional<Greeting> FindGreeting() const;
};

optional<Greeting> Greetings::FindGreeting() const {
    auto hour_holder = mmotd::chrono::io::get_current_hour();
    auto hour = HourType{5};
    if (!hour_holder) {
        LOG_ERROR("unable to query for the current hour, defaulting to 5am");
    } else {
        hour = *hour_holder;
        LOG_VERBOSE("the current hour is {}", hour);
    }
    if (value_in_range(hour, MORNING_GREETING.begin(), MORNING_GREETING.end())) {
        return MORNING_GREETING;
    } else if (value_in_range(hour, AFTERNOON_GREETING.begin(), AFTERNOON_GREETING.end())) {
        return AFTERNOON_GREETING;
    } else if (value_in_range(hour, EVENING_GREETING.begin(), EVENING_GREETING.end())) {
        return EVENING_GREETING;
    } else {
        MMOTD_CHECKS(value_outside_range(hour, NIGHT_GREETING.begin(), NIGHT_GREETING.end()),
                     fmt::format(FMT_STRING("hour value={} did not find it's way into any block of time"), hour));
        return NIGHT_GREETING;
    }
}

string Greetings::GetTimeOfDayGreeting() const {
    auto greeting_wrapper = FindGreeting();
    return greeting_wrapper ? greeting_wrapper.value().GetTimeOfDayGreeting() : string{"Hello"};
}

string Greetings::GetTimeOfDayEmoji() const {
    auto greeting_wrapper = FindGreeting();
    return greeting_wrapper ? greeting_wrapper.value().GetRandomEmoji() : string{"🕹"};
}

string Greetings::GetLocalDateTime() const {
    return mmotd::chrono::io::to_string(std::chrono::system_clock::now(), "%a, %d-%h-%Y %I:%M%p %Z");
}

} // namespace

namespace mmotd::information {

void General::FindInformation() {
    auto user_info = mmotd::core::GetUserInformation();
    if (user_info.empty()) {
        return;
    }

    auto username = GetInfoTemplate(InformationId::ID_GENERAL_USER_NAME);
    username.SetValueArgs(user_info.full_name);
    AddInformation(username);

    auto greeting_info = GetInfoTemplate(InformationId::ID_GENERAL_GREETING);
    greeting_info.SetValue(Greetings{}.GetTimeOfDayGreeting());
    AddInformation(greeting_info);

    auto local_date_time = GetInfoTemplate(InformationId::ID_GENERAL_LOCAL_DATE_TIME);
    local_date_time.SetValue(Greetings{}.GetLocalDateTime());
    AddInformation(local_date_time);

    auto emoji_info = GetInfoTemplate(InformationId::ID_GENERAL_LOCAL_TIME_EMOJI);
    emoji_info.SetValue(Greetings{}.GetTimeOfDayEmoji());
    AddInformation(emoji_info);
}

} // namespace mmotd::information
