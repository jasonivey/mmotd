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
    auto processes_count_holder = mmotd::platform::GetProcessCount();
    auto processes_count = processes_count_holder ? *processes_count_holder : size_t{0};

    auto obj = GetInfoTemplate(InformationId::ID_PROCESSES_PROCESS_COUNT);
    obj.SetValue(processes_count);
    AddInformation(obj);

    return true;
}

} // namespace mmotd::information
