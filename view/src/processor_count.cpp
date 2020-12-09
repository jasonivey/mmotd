// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/processor_count.h"

#include <memory>

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkProcessorCountProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_shared<mmotd::ProcessorCount>(); });

optional<string> mmotd::ProcessorCount::QueryInformation() {
    auto processor_count_wrapper = ComputerInformation::Instance().GetInformation("processor count");
    if (!processor_count_wrapper) {
        PLOG_ERROR << "processor count was not returned";
        return nullopt;
    } else if ((*processor_count_wrapper).size() != 1) {
        PLOG_ERROR << format("the processor count was returned but there were {} values",
                             (*processor_count_wrapper).size());
        return nullopt;
    } else if ((*processor_count_wrapper).front().empty()) {
        PLOG_ERROR << "the processor count was returned but it was empty";
        return nullopt;
    }
    auto processor_count = (*processor_count_wrapper).front();
    return make_optional(processor_count);
}

string mmotd::ProcessorCount::GetName() const {
    return "processor count";
}
