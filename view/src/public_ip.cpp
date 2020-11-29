// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/public_ip.h"

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkPublicIpProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::PublicIp>(); });

optional<string> mmotd::PublicIp::QueryInformation() {
    auto public_ip_info = ComputerInformation::Instance().GetInformation("public ip");
    if (!public_ip_info) {
        PLOG_INFO << "no public ips were returned from computer information api";
        return nullopt;
    } else if ((*public_ip_info).size() != 1) {
        PLOG_INFO << format("a public ip was returned but the list contained {} items", (*public_ip_info).size());
        return nullopt;
    } else if ((*public_ip_info).front().empty()) {
        PLOG_ERROR << "the public ip was returned but it was empty";
        return nullopt;
    }
    auto value = (*public_ip_info).front();
    return make_optional(value);
}

string mmotd::PublicIp::GetName() const {
    return "public ip";
}
