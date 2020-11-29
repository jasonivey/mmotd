// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/computer_information_provider_factory.h"
#include "view/include/random_quote.h"

using namespace std;

bool gLinkRandomQuoteProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::RandomQuote>(); });

optional<string> mmotd::RandomQuote::QueryInformation() {
    return nullopt;
}

string mmotd::RandomQuote::GetName() const {
    return "random quote";
}
