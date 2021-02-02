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

bool gLinkMemoryUsage = false;

namespace mmotd::information {

static const bool memory_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::Memory>(); });

bool Memory::FindInformation() {
    using mmotd::platform::Detail, mmotd::platform::Details;

    if (auto details = mmotd::platform::GetMemoryDetails(); !details.empty()) {
        for (const auto &[name, value] : details) {
            if (name == "total") {
                auto obj = GetInfoTemplate(InformationId::ID_MEMORY_USAGE_TOTAL);
                obj.information = value;
                AddInformation(obj);
            } else if (name == "percent") {
                auto obj = GetInfoTemplate(InformationId::ID_MEMORY_USAGE_PERCENT_USED);
                obj.information = value;
                AddInformation(obj);
            } else if (name == "free") {
                auto obj = GetInfoTemplate(InformationId::ID_MEMORY_USAGE_FREE);
                obj.information = value;
                AddInformation(obj);
            }
        }
    }
    return true;
}

} // namespace mmotd::information
