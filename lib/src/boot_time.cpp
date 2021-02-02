// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/chrono_io.h"
#include "common/include/information.h"
#include "common/include/information_definitions.h"
#include "common/include/posix_error.h"
#include "lib/include/boot_time.h"
#include "lib/include/computer_information.h"
#include "lib/include/platform/boot_time.h"

#include <chrono>
#include <ctime>
#include <iterator>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <boost/preprocessor/cat.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkBootTime = false;

namespace mmotd::information {

static const bool boot_time_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::BootTime>(); });

bool BootTime::FindInformation() {
    if (auto boot_time_wrapper = mmotd::platform::GetBootTime(); !boot_time_wrapper) {
        return false;
    } else {
        auto info = GetInfoTemplate(InformationId::ID_BOOT_TIME_BOOT_TIME);
        info.information = *boot_time_wrapper;
        AddInformation(info);
        return true;
    }
}

} // namespace mmotd::information
