// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/lastlog.h"
#include "lib/include/platform/lastlog.h"

#include <memory>
#include <string>

using namespace std;

bool gLinkLastLog = false;

namespace mmotd {

static const bool last_log_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::LastLog>(); });

bool LastLog::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        details_ = mmotd::platform::GetLastLogDetails();
        return !details_.empty();
    }
    return has_queried;
}

std::optional<mmotd::ComputerValues> LastLog::GetInformation() const {
    return !details_.empty() ? make_optional(details_) : nullopt;
}

} // namespace mmotd
