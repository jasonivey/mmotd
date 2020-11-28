// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace mmotd {

class InformationProvider;
using InformationProviderPtr = std::unique_ptr<InformationProvider>;
using InformationProviderCreator = std::function<InformationProviderPtr()>;

bool RegisterInformationProvider(InformationProviderCreator creator);

class ComputerInformation {
public:
    virtual ~ComputerInformation() = default;
    ComputerInformation(const ComputerInformation &other) = default;
    ComputerInformation(ComputerInformation &&other) noexcept = default;
    ComputerInformation &operator=(const ComputerInformation &other) = default;
    ComputerInformation &operator=(ComputerInformation &&other) noexcept = default;

    static ComputerInformation &Instance();

    std::optional<std::vector<std::string>> GetInformation(std::string_view name) const;
    std::vector<std::tuple<std::string, std::string>> GetAllInformation() const;

private:
    ComputerInformation();

    using InformationProviders = std::vector<InformationProviderPtr>;
    InformationProviders GetInformationProviders();
    void SetInformationProviders();

    using NameAndValue = std::tuple<std::string, std::string>;
    using Information = std::vector<NameAndValue>;

    bool IsInformationCached() const;
    void CacheAllInformation() const;

    InformationProviders information_providers_;
    mutable Information information_cache_;
};

} // namespace mmotd
