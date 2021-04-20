// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/platform/package_management.h"
#include "lib/include/package_management.h"

#include <cstdint>
#include <vector>

#include <fmt/format.h>

using fmt::format;
using namespace std;

bool gLinkPackageManagementInfo = false;

namespace mmotd::information {

static const bool package_management_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::PackageManagement>(); });

bool PackageManagement::FindInformation() {
    auto update = mmotd::platform::GetPackageManagementUpdate();

    auto package_management = GetInfoTemplate(InformationId::ID_PACKAGE_MANAGEMENT_UPDATE);
    package_management.SetValueArgs(update);
    AddInformation(package_management);

    return true;
}

} // namespace mmotd::information
