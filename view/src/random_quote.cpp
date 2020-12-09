// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/random_quote.h"

#include <memory>

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkRandomQuoteProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_shared<mmotd::RandomQuote>(); });

optional<string> mmotd::RandomQuote::QueryInformation() {
    auto fortune_wrapper = ComputerInformation::Instance().GetInformation("fortune");
    if (!fortune_wrapper || (*fortune_wrapper).empty()) {
        PLOG_ERROR << "unable to find fortune quote";
        return nullopt;
    } else if ((*fortune_wrapper).size() != 1) {
        PLOG_ERROR << format("able to query a fortune successfully but {} items were returned",
                             (*fortune_wrapper).size());
        return nullopt;
    } else if ((*fortune_wrapper).front().empty()) {
        PLOG_ERROR << "able to query a fortune successfully the fortune was empty";
        return nullopt;
    }
    return make_optional((*fortune_wrapper).front());
}

string mmotd::RandomQuote::GetName() const {
    return "random quote";
}
