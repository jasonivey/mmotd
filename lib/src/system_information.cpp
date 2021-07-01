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

void SystemInformation::FindInformation() {
    auto details = mmotd::platform::GetSystemInformationDetails();
    if (!details.empty()) {
        CreateInformationObjects(details);
    }
}

void SystemInformation::CreateInformationObjects(const mmotd::platform::SystemDetails &details) {
    auto host_name = GetInfoTemplate(InformationId::ID_SYSTEM_INFORMATION_HOST_NAME);
    host_name.SetValueArgs(details.host_name);
    AddInformation(host_name);

    auto computer_name = GetInfoTemplate(InformationId::ID_SYSTEM_INFORMATION_COMPUTER_NAME);
    computer_name.SetValueArgs(details.computer_name);
    AddInformation(computer_name);

    auto kernel_version = GetInfoTemplate(InformationId::ID_SYSTEM_INFORMATION_KERNEL_VERSION);
    kernel_version.SetValueArgs(details.kernel_version);
    AddInformation(kernel_version);

    auto kernel_release = GetInfoTemplate(InformationId::ID_SYSTEM_INFORMATION_KERNEL_RELEASE);
    kernel_release.SetValueArgs(details.kernel_release);
    AddInformation(kernel_release);

    auto kernel_type = GetInfoTemplate(InformationId::ID_SYSTEM_INFORMATION_KERNEL_TYPE);
    kernel_type.SetValueArgs(details.kernel_type);
    AddInformation(kernel_type);

    auto architecture_type = GetInfoTemplate(InformationId::ID_SYSTEM_INFORMATION_ARCHITECTURE);
    architecture_type.SetValueArgs(details.architecture_type);
    AddInformation(architecture_type);

    auto platform_version = GetInfoTemplate(InformationId::ID_SYSTEM_INFORMATION_PLATFORM_VERSION);
    platform_version.SetValueArgs(details.platform_version);
    AddInformation(platform_version);

    auto platform_name = GetInfoTemplate(InformationId::ID_SYSTEM_INFORMATION_PLATFORM_NAME);
    platform_name.SetValueArgs(details.platform_name);
    AddInformation(platform_name);
}

} // namespace mmotd::information
