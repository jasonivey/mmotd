// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/active_network_interfaces.h"
#include "view/include/computer_information_provider_factory.h"

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <iterator>
#include <plog/Log.h>
#include <unordered_map>
#include <vector>

using namespace std;
using fmt::format;

bool gLinkActiveNetworkInterfaces = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::ActiveNetworkInterfaces>(); });

optional<string> mmotd::ActiveNetworkInterfaces::QueryInformation() {
    auto network_infos = ComputerInformation::Instance().GetInformation("network info");
    if (!network_infos) {
        PLOG_INFO << "no active network interfaces found";
        return nullopt;
    }
    auto network_interfaces = unordered_map<string, vector<string>>{};
    for (auto network_value : *network_infos) {
        auto split_network = vector<string>{};
        // ☞ WHITE RIGHT POINTING INDEX, Unicode: U+261E, UTF-8: E2 98 9E
        boost::split(split_network, network_value, boost::is_any_of("☞"), boost::token_compress_on);
        if (split_network.size() != 2) {
            PLOG_INFO << format("network value was in an unexpected form: {}", network_value);
            continue;
        }
        auto interface_name = split_network.front();
        auto value = split_network.back();
        auto i = network_interfaces.find(interface_name);
        auto *network_details = i == end(network_interfaces) ? nullptr : &(i->second);
        if (i == end(network_interfaces)) {
            i = network_interfaces.insert(cbegin(network_interfaces), {interface_name, vector<string>{}});
            network_details = &(i->second);
        }
        (*network_details).push_back(value);
    }
    auto details = string{};
    for (auto [interface_name, values] : network_interfaces) {
        details += format("{}{}: ", details.empty() ? "" : ", ", interface_name);
        for (const auto &value : values) {
            if (boost::ends_with(details, ": ")) {
                details += value;
            } else {
                details += format(", {}", value);
            }
        }
    }
    return make_optional(details);
}

string mmotd::ActiveNetworkInterfaces::GetName() const {
    return "active-network-interfaces";
}
