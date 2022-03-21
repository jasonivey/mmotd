// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/logging.h"
#include "lib/include/computer_information.h"
#include "lib/include/information_provider.h"

#include <algorithm>
#include <iterator>
#include <thread>

#include <boost/asio.hpp>
#include <fmt/format.h>

using namespace std;
using fmt::format;

namespace {

using InformationProviderCreators = vector<mmotd::information::InformationProviderCreator>;

InformationProviderCreators &GetInformationProviderCreators() {
    static InformationProviderCreators computer_information_provider_creators;
    return computer_information_provider_creators;
}

} // namespace

namespace mmotd::information {

ComputerInformation::ComputerInformation() : information_providers_(), information_cache_() {
    SetInformationProviders();
}

ComputerInformation &ComputerInformation::Instance() {
    static auto computer_information = ComputerInformation{};
    return computer_information;
}

void ComputerInformation::SetInformationProviders() {
    auto &creators = GetInformationProviderCreators();
    information_providers_.resize(creators.size());
    transform(begin(creators), end(creators), begin(information_providers_), [](auto &creator) { return creator(); });
    LOG_INFO("created {} information providers", information_providers_.size());
}

bool RegisterInformationProvider(InformationProviderCreator creator) {
    auto &information_provider_creators = GetInformationProviderCreators();
    information_provider_creators.emplace_back(creator);
    return true;
}

optional<Information> ComputerInformation::FindInformation(InformationId id) const {
    PRECONDITIONS(IsInformationCached(), "information cache is not initialized -- call GetAllInformations() first");
    if (!information_cache_.contains(id) || empty(information_cache_.at(id))) {
        return nullopt;
    } else {
        return information_cache_.at(id).front();
    }
}

Informations &ComputerInformation::GetAllInformations() {
    if (!IsInformationCached()) {
        CacheInformation();
    }
    return information_cache_;
}

vector<Information> ComputerInformation::GetInformations() {
    if (!IsInformationCached()) {
        CacheInformation();
    }
    auto informations = vector<Information>{};
    for (auto &[_, infos] : information_cache_) {
        copy(begin(infos), end(infos), back_inserter(informations));
    }
    return informations;
}

bool ComputerInformation::IsInformationCached() const {
    return !information_cache_.empty();
}

void ComputerInformation::CacheInformationAsync() {
    auto thread_pool = boost::asio::thread_pool{std::thread::hardware_concurrency()};
    for (auto &&provider : information_providers_) {
        boost::asio::post(thread_pool, [&provider]() { provider->LookupInformation(); });
    }
    thread_pool.join();
}

void ComputerInformation::CacheInformationSerial() {
    for (auto &&provider : information_providers_) {
        provider->LookupInformation();
    }
}

void ComputerInformation::CacheInformation() {
    if (IsInformationCached()) {
        return;
    }
#if defined(MMOTD_ASYNC_DISABLED)
    CacheInformationSerial();
#else
    CacheInformationAsync();
#endif
    for (const auto &provider : information_providers_) {
        const auto &informations = provider->GetInformations();
        for (auto [id, infos] : informations) {
            if (!information_cache_.contains(id)) {
                information_cache_.emplace(move(id), move(infos));
            } else {
                auto &existing_infos = information_cache_.at(id);
                copy(begin(infos), end(infos), back_inserter(existing_infos));
            }
        }
    }
}

} // namespace mmotd::information
