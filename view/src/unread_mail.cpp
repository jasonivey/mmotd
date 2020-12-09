// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/computer_information_provider_factory.h"
#include "view/include/unread_mail.h"

#include <memory>

using namespace std;

bool gLinkUnreadMailProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_shared<mmotd::UnreadMail>(); });

optional<string> mmotd::UnreadMail::QueryInformation() {
    return nullopt;
}

string mmotd::UnreadMail::GetName() const {
    return "unread mail";
}
