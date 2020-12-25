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

namespace mmotd {

static const bool memory_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::Memory>(); });

bool Memory::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        details_ = mmotd::platform::GetMemoryDetails();
        return !details_.empty();
    }
    return has_queried;
}

optional<mmotd::ComputerValues> Memory::GetInformation() const {
    return !details_.empty() ? make_optional(details_) : nullopt;
}

} // namespace mmotd
