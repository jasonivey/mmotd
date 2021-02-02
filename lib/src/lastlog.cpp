// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/lastlog.h"
#include "lib/include/platform/lastlog.h"

#include <memory>
#include <string>

using namespace std;

bool gLinkLastLog = false;

namespace mmotd::information {

static const bool last_log_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::LastLog>(); });

bool LastLog::FindInformation() {
    for (auto [name, value] : mmotd::platform::GetLastLogDetails()) {
        if (name == "last log") {
            auto last_log = GetInfoTemplate(InformationId::ID_LAST_LOGIN_LOGIN_SUMMARY);
            last_log.information = value;
            AddInformation(last_log);
        } else if (name == "log in") {
            auto log_in = GetInfoTemplate(InformationId::ID_LAST_LOGIN_LOGIN_TIME);
            log_in.information = value;
            AddInformation(log_in);
        } else if (name == "log out") {
            auto log_out = GetInfoTemplate(InformationId::ID_LAST_LOGIN_LOGOUT_TIME);
            log_out.information = value;
            AddInformation(log_out);
        }
    }
    return true;
}

} // namespace mmotd::information
