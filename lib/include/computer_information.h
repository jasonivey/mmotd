// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"
#include "common/include/information_definitions.h"

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace mmotd::information {

class InformationProvider;
using InformationProviderPtr = std::unique_ptr<InformationProvider>;
using InformationProviderCreator = std::function<InformationProviderPtr()>;

bool RegisterInformationProvider(InformationProviderCreator creator);

class Informations;

class ComputerInformation {
public:
    NO_CONSTRUCTOR_DEFAULT_COPY_MOVE_OPERATORS_DESTRUCTOR(ComputerInformation);

    static ComputerInformation &Instance();

    std::optional<Information> FindInformation(InformationId id) const;
    const Informations &GetAllInformation() const;

private:
    ComputerInformation();

    using InformationProviders = std::vector<InformationProviderPtr>;
    InformationProviders GetInformationProviders();
    void SetInformationProviders();

    bool IsInformationCached() const;
    void CacheAllInformation() const;
    void CacheAllInformationAsync() const;
    void CacheAllInformationSerial() const;

    InformationProviders information_providers_;
    mutable Informations information_cache_;
};

} // namespace mmotd::information
