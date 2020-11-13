// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/computer_information_provider.h"
#include "view/include/computer_information_provider_factory.h"

using namespace std;

using ComputerInformationProviderCreators = vector<mmotd::ComputerInformationProviderCreator>;

static ComputerInformationProviderCreators &GetComputerInformationProviderCreators() {
    static ComputerInformationProviderCreators computer_information_provider_creators;
    return computer_information_provider_creators;
}

bool mmotd::RegisterComputerInformationProvider(ComputerInformationProviderCreator &&creator) {
    auto &computer_information_provider_creators = GetComputerInformationProviderCreators();
    computer_information_provider_creators.emplace_back(creator);
    return true;
}

mmotd::ComputerInformationProviders mmotd::GetComputerInformationProviders() {
    auto &computer_information_provider_creators = GetComputerInformationProviderCreators();
    auto computer_information_providers = mmotd::ComputerInformationProviders{};
    for (auto &&creator : computer_information_provider_creators) {
        computer_information_providers.emplace_back(creator());
    }
    return computer_information_providers;
}
