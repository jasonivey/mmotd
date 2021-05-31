// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/human_size.h"
#include "common/include/logging.h"
#include "common/include/posix_error.h"
#include "lib/include/computer_information.h"
#include "lib/include/memory.h"
#include "lib/include/platform/memory.h"

#include <fmt/format.h>
#include <scope_guard.hpp>

using fmt::format;
using namespace std;
using mmotd::algorithm::string::to_human_size;

bool gLinkMemoryUsage = false;

namespace mmotd::information {

static const bool memory_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::Memory>(); });

void Memory::FindInformation() {
    auto details = mmotd::platform::GetMemoryDetails();

    auto total = GetInfoTemplate(InformationId::ID_MEMORY_USAGE_TOTAL);
    total.SetValueArgs(details.total);
    AddInformation(total);

    auto free = GetInfoTemplate(InformationId::ID_MEMORY_USAGE_FREE);
    free.SetValueArgs(details.free);
    AddInformation(free);

    auto percent_used = GetInfoTemplate(InformationId::ID_MEMORY_USAGE_PERCENT_USED);
    percent_used.SetValueArgs(details.percent_used, to_human_size(details.total));
    AddInformation(percent_used);
}

} // namespace mmotd::information
