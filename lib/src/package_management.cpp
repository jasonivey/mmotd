// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/package_management.h"
#include "lib/include/platform/package_management.h"

#include <cstdint>
#include <vector>

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkPackageManagementInfo = false;

namespace mmotd::information {

static const bool package_management_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::PackageManagement>(); });

bool PackageManagement::FindInformation() {
    auto update_details = platform::package_management::GetUpdateDetails();
    if (!empty(update_details)) {
        auto update_details_info = GetInfoTemplate(InformationId::ID_PACKAGE_MANAGEMENT_UPDATE_DETAILS);
        update_details_info.SetValueArgs(update_details);
        PLOG_VERBOSE << format(FMT_STRING("set the value of package management update details: {}"), update_details);
        AddInformation(update_details_info);
    }
    auto reboot_required = platform::package_management::GetRebootRequired();
    if (!empty(reboot_required)) {
        auto reboot_required_info = GetInfoTemplate(InformationId::ID_PACKAGE_MANAGEMENT_REBOOT_REQUIRED);
        reboot_required_info.SetValueArgs(reboot_required);
        PLOG_VERBOSE << format(FMT_STRING("set the value of package management reboot required: {}"), reboot_required);
        AddInformation(reboot_required_info);
    }
    return true;
}

} // namespace mmotd::information
