// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/computer_information_provider.h"
#include "view/include/computer_information_provider_factory.h"

#include <fmt/format.h>
#include <plog/Log.h>

using namespace std;

using ComputerInformationProviderCreators = vector<mmotd::ComputerInformationProviderCreator>;

static ComputerInformationProviderCreators &GetComputerInformationProviderCreators() {
    static ComputerInformationProviderCreators computer_information_provider_creators;
    return computer_information_provider_creators;
}

bool mmotd::RegisterComputerInformationProvider(ComputerInformationProviderCreator creator) {
    auto &computer_information_provider_creators = GetComputerInformationProviderCreators();
    computer_information_provider_creators.emplace_back(creator);
    // cout << "total creators " << computer_information_provider_creators.size() << endl;
    return true;
}

mmotd::ComputerInformationProviders mmotd::GetComputerInformationProviders() {
    auto computer_information_providers = mmotd::ComputerInformationProviders{};
    for (auto creator : GetComputerInformationProviderCreators()) {
        computer_information_providers.emplace_back(creator());
    }
    PLOG_ERROR << fmt::format("returning {} total creators", computer_information_providers.size());
    return computer_information_providers;
}
