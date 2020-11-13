// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/computer_information_provider_factory.h"
#include "view/include/unread_mail.h"

using namespace std;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::UnreadMail>(); });

optional<string> mmotd::UnreadMail::QueryInformation() {
    return nullopt;
}

string mmotd::UnreadMail::GetName() const {
    return "unread-mail";
}
