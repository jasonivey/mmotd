// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/chrono_io.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/header.h"

#include <chrono>

#include <fmt/format.h>

using fmt::format;
using namespace std;

bool gLinkHeaderProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::Header>(); });

optional<string> mmotd::Header::QueryInformation() { // System information as of Sun, 06-Dec-2020 🌦 06:14:23am MST
    auto right_now = std::chrono::system_clock::now();
    return make_optional(
        mmotd::chrono::io::to_string(right_now, "System information as of {:%a, %d-%h-%Y %I:%M:%S%p %Z}"));
}

string mmotd::Header::GetName() const {
    return "header";
}
