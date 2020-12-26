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

namespace mmotd {

static const bool processes_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::Processes>(); });

bool Processes::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        auto count_wrapper = mmotd::platform::GetProcessCount();
        if (count_wrapper) {
            details_.push_back(make_tuple("processes", format("count: {}", *count_wrapper)));
        }
        return !details_.empty();
    }
    return has_queried;
}

std::optional<mmotd::ComputerValues> Processes::GetInformation() const {
    return details_.empty() ? nullopt : make_optional(details_);
}

} // namespace mmotd
