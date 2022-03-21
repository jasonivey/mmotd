// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"
#include "common/include/information.h"

#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace mmotd::information {

class InformationProvider;
using InformationProviderPtr = std::unique_ptr<InformationProvider>;
using InformationProviderCreator = std::function<InformationProviderPtr()>;

bool RegisterInformationProvider(InformationProviderCreator creator);

class ComputerInformation {
public:
    NO_CONSTRUCTOR_DEFAULT_COPY_MOVE_OPERATORS_DESTRUCTOR(ComputerInformation);

    static ComputerInformation &Instance();

    // Made public for future use -- currently this is called internally by GetAllInformations & GetInformations
    void CacheInformation();

    // This method does not call the CacheInformation method
    std::optional<Information> FindInformation(InformationId id) const;

    // These methods DO call the CacheInformation method
    Informations &GetAllInformations();
    std::vector<Information> GetInformations();

private:
    ComputerInformation();

    using InformationProviders = std::vector<InformationProviderPtr>;
    InformationProviders GetInformationProviders();
    void SetInformationProviders();

    bool IsInformationCached() const;
    void CacheInformationAsync();
    void CacheInformationSerial();

    InformationProviders information_providers_;
    Informations information_cache_;
};

} // namespace mmotd::information
