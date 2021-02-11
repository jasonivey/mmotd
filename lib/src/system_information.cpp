// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/platform/system_information.h"
#include "lib/include/system_information.h"

#include <optional>
#include <tuple>
#include <vector>

using namespace std;

bool gLinkSystemInformation = false;

namespace mmotd::information {

static const bool system_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::SystemInformation>(); });

bool SystemInformation::FindInformation() {
    if (auto details = mmotd::platform::GetSystemInformationDetails(); !details.empty()) {
        CreateInformationObjects(details);
        return true;
    }
    return false;
}

void SystemInformation::CreateInformationObjects(const mmotd::platform::SystemInformationDetails &details) {
    for (const auto &[name, value] : details) {
        if (name == "host name") {
            auto hostname = GetInfoTemplate(InformationId::ID_SYSTEM_INFORMATION_HOST_NAME);
            hostname.SetValue(value);
            AddInformation(hostname);

            auto computername = GetInfoTemplate(InformationId::ID_SYSTEM_INFORMATION_COMPUTER_NAME);
            auto i = value.find('.');
            computername.SetValue(value.substr(0, i));
            AddInformation(computername);
        } else if (name == "kernel version") {
            auto obj = GetInfoTemplate(InformationId::ID_SYSTEM_INFORMATION_KERNEL_VERSION);
            obj.SetValue(value);
            AddInformation(obj);
        } else if (name == "kernel release") {
            auto obj = GetInfoTemplate(InformationId::ID_SYSTEM_INFORMATION_KERNEL_RELEASE);
            obj.SetValue(value);
            AddInformation(obj);
        } else if (name == "kernel type") {
            auto obj = GetInfoTemplate(InformationId::ID_SYSTEM_INFORMATION_KERNEL_TYPE);
            obj.SetValue(value);
            AddInformation(obj);
        } else if (name == "architecture") {
            auto obj = GetInfoTemplate(InformationId::ID_SYSTEM_INFORMATION_ARCHITECTURE);
            obj.SetValue(value);
            AddInformation(obj);
        } else if (name == "byte order") {
            auto obj = GetInfoTemplate(InformationId::ID_SYSTEM_INFORMATION_BYTEORDER);
            obj.SetValue(value);
            AddInformation(obj);
        } else if (name == "platform version") {
            auto obj = GetInfoTemplate(InformationId::ID_SYSTEM_INFORMATION_PLATFORM_VERSION);
            obj.SetValue(value);
            AddInformation(obj);
        } else if (name == "platform name") {
            auto obj = GetInfoTemplate(InformationId::ID_SYSTEM_INFORMATION_PLATFORM_NAME);
            obj.SetValue(value);
            AddInformation(obj);
        }
    }
}

} // namespace mmotd::information
