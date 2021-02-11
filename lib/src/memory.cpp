// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/human_size.h"
#include "common/include/posix_error.h"
#include "lib/include/computer_information.h"
#include "lib/include/memory.h"
#include "lib/include/platform/memory.h"

#include <fmt/format.h>
#include <plog/Log.h>
#include <scope_guard.hpp>

using fmt::format;
using namespace std;
using mmotd::algorithm::string::to_human_size;

bool gLinkMemoryUsage = false;

namespace mmotd::information {

static const bool memory_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::Memory>(); });

bool Memory::FindInformation() {
    auto details = mmotd::platform::GetMemoryDetails();

    auto total = GetInfoTemplate(InformationId::ID_MEMORY_USAGE_TOTAL);
    total.SetValue(details.total);
    AddInformation(total);

    auto free = GetInfoTemplate(InformationId::ID_MEMORY_USAGE_FREE);
    free.SetValue(details.free);
    AddInformation(free);

    auto percent_used = GetInfoTemplate(InformationId::ID_MEMORY_USAGE_PERCENT_USED);
    percent_used.SetValue(details.percent_used, to_human_size(details.total));
    AddInformation(percent_used);

    return true;
}

} // namespace mmotd::information
