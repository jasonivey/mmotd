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
    auto lastlog_details = mmotd::platform::GetLastLogDetails();

    auto last_log = GetInfoTemplate(InformationId::ID_LAST_LOGIN_LOGIN_SUMMARY);
    last_log.SetValueArgs(lastlog_details.summary);
    AddInformation(last_log);

    auto log_in = GetInfoTemplate(InformationId::ID_LAST_LOGIN_LOGIN_TIME);
    log_in.SetValueArgs(lastlog_details.log_in);
    AddInformation(log_in);

    auto log_out = GetInfoTemplate(InformationId::ID_LAST_LOGIN_LOGOUT_TIME);
    if (lastlog_details.log_out == std::chrono::system_clock::time_point{}) {
        log_out.SetValueArgs("still logged in");
    } else {
        // steal the date format string from the InformationId::ID_LAST_LOGIN_LOGIN_TIME
        log_out.SetValueFormat(log_in.GetFormat(), lastlog_details.log_out);
    }
    AddInformation(log_out);

    return true;
}

} // namespace mmotd::information
