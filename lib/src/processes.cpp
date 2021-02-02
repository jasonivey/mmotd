// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/platform/processes.h"
#include "lib/include/processes.h"

#include <cstdint>
#include <vector>

#include <fmt/format.h>

using fmt::format;
using namespace std;

bool gLinkProcessesInfo = false;

namespace mmotd::information {

static const bool processes_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::Processes>(); });

bool Processes::FindInformation() {
    if (auto count_wrapper = mmotd::platform::GetProcessCount(); count_wrapper) {
        auto obj = GetInfoTemplate(InformationId::ID_PROCESSES_PROCESS_COUNT);
        obj.information = *count_wrapper;
        AddInformation(obj);
        return true;
    }
    return false;
}

} // namespace mmotd::information
