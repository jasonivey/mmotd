// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/algorithm.h"
#include "lib/include/computer_information.h"
#include "lib/include/information_provider.h"

#include <algorithm>
#include <fmt/format.h>
#include <iterator>
#include <plog/Log.h>

using namespace std;
using mmotd::algorithms::transform_if;

namespace mmotd {

namespace detail {

using InformationProviderCreators = vector<InformationProviderCreator>;

InformationProviderCreators &GetInformationProviderCreators() {
    static InformationProviderCreators computer_information_provider_creators;
    return computer_information_provider_creators;
}

} // namespace detail

ComputerInformation::ComputerInformation() : information_providers_(), information_cache_() {
}

ComputerInformation &ComputerInformation::Instance() {
    static auto computer_information = ComputerInformation{};
    computer_information.SetInformationProviders();
    return computer_information;
}

void ComputerInformation::SetInformationProviders() {
    static bool already_done = false;
    if (already_done) {
        return;
    }
    auto &creators = detail::GetInformationProviderCreators();
    information_providers_ = InformationProviders(creators.size());
    transform(begin(creators), end(creators), begin(information_providers_), [](auto &creator) { return creator(); });
    PLOG_INFO << fmt::format("created {} information providers", information_providers_.size());
    already_done = true;
}

bool RegisterInformationProvider(InformationProviderCreator creator) {
    auto &information_provider_creators = detail::GetInformationProviderCreators();
    information_provider_creators.emplace_back(creator);
    return true;
}

optional<vector<string>> ComputerInformation::GetInformation(string_view name) const {
    if (!IsInformationCached()) {
        CacheAllInformation();
    }
    auto info = vector<string>{};
    transform_if(
        begin(information_cache_),
        end(information_cache_),
        back_inserter(info),
        [name](const auto &name_value) { return std::get<0>(name_value) == name; },
        [](const auto &name_value) { return std::get<1>(name_value); });
    return info.empty() ? nullopt : make_optional(info);
}

vector<tuple<string, string>> ComputerInformation::GetAllInformation() const {
    if (!IsInformationCached()) {
        CacheAllInformation();
    }
    return information_cache_;
}

bool ComputerInformation::IsInformationCached() const {
    return !information_cache_.empty();
}

void ComputerInformation::CacheAllInformation() const {
    for (auto &&provider : information_providers_) {
        if (!provider->QueryInformation()) {
            PLOG_ERROR << fmt::format("unable to query information for {}", provider->GetName());
            continue;
        }
        auto information = provider->GetInformation();
        if (!information) {
            PLOG_ERROR << fmt::format("successfully queried information for {} but nothing was returned",
                                      provider->GetName());
            continue;
        }
        //make_move_iterator
        auto values = information.value();
        information_cache_.insert(end(information_cache_), begin(values), end(values));
    }
}

} // namespace mmotd
