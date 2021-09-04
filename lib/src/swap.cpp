// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/human_size.h"
#include "lib/include/computer_information.h"
#include "lib/include/platform/swap.h"
#include "lib/include/swap.h"

#include <fmt/format.h>

using fmt::format;
using namespace std;
using mmotd::algorithm::string::to_human_size;

bool gLinkSwapUsage = false;

namespace mmotd::information {

static const bool swap_usage_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::Swap>(); });

void Swap::FindInformation() {
    auto details = mmotd::platform::GetSwapDetails();

    auto total = GetInfoTemplate(InformationId::ID_SWAP_USAGE_TOTAL);
    total.SetValueArgs(to_human_size(details.total));
    AddInformation(total);

    auto free = GetInfoTemplate(InformationId::ID_SWAP_USAGE_FREE);
    free.SetValueArgs(details.free);
    AddInformation(free);

    auto percent_used = GetInfoTemplate(InformationId::ID_SWAP_USAGE_PERCENT_USED);
    percent_used.SetValueArgs(details.percent_used);
    AddInformation(percent_used);

    auto encrypted = GetInfoTemplate(InformationId::ID_SWAP_USAGE_ENCRYPTED);
    encrypted.SetValueArgs(details.encrypted ? "[encrypted]" : "[un-encrypted]");
    AddInformation(encrypted);

    auto summary = GetInfoTemplate(InformationId::ID_SWAP_USAGE_SUMMARY);
    auto summary_str = string{"encrypted"};
    if (!details.encrypted) {
        summary_str = format(FMT_STRING("{:.01f}% of {}"), details.percent_used, to_human_size(details.total));
    }
    summary.SetValue(summary_str);
    AddInformation(summary);
}

} // namespace mmotd::information
