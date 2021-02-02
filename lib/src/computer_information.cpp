// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/algorithm.h"
#include "common/include/information_definitions.h"
#include "common/include/information_objects.h"
#include "lib/include/computer_information.h"
#include "lib/include/information_provider.h"

#include <algorithm>
#include <iterator>
#include <thread>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

using namespace std;
using fmt::format;
using mmotd::algorithms::transform_if;

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
    information_providers_ = InformationProviders(creators.size());
    transform(begin(creators), end(creators), begin(information_providers_), [](auto &creator) { return creator(); });
    PLOG_INFO << format("created {} information providers", information_providers_.size());
}

bool RegisterInformationProvider(InformationProviderCreator creator) {
    auto &information_provider_creators = GetInformationProviderCreators();
    information_provider_creators.emplace_back(creator);
    return true;
}

optional<Information> ComputerInformation::FindInformation(InformationId id) const {
    if (!IsInformationCached()) {
        CacheAllInformation();
    }
    auto i = find_if(begin(information_cache_), end(information_cache_), [id](const Information &info) {
        return id == info.information_id;
    });
    return i == end(information_cache_) ? nullopt : make_optional(*i);
}

const Informations &ComputerInformation::GetAllInformation() const {
    if (!IsInformationCached()) {
        CacheAllInformation();
    }
    return information_cache_;
}

bool ComputerInformation::IsInformationCached() const {
    return !information_cache_.empty();
}

void ComputerInformation::CacheAllInformation() const {
#if defined(MMOTD_ASYNC_DISABLED)
    for (auto &&provider : information_providers_) {
        if (!provider->QueryInformation()) {
            PLOG_ERROR << format("unable to query information for {}", provider->GetName());
            continue;
        }
        auto information_wrapper = provider->GetInformation();
        if (!information_wrapper) {
            PLOG_ERROR << format("successfully queried information for {} but nothing was returned",
                                 provider->GetName());
            continue;
        }
        auto values = information_wrapper.value();
        copy(begin(values), end(values), back_inserter(information_cache_));
    }
#else
    auto thread_pool = boost::asio::thread_pool{std::thread::hardware_concurrency()};
    for (auto &&provider : information_providers_) {
        boost::asio::post(thread_pool, boost::bind(&InformationProvider::LookupInformation, provider.get()));
    }
    thread_pool.join();
    for (const auto &provider : information_providers_) {
        const auto &informations = provider->GetInformations();
        copy(begin(informations), end(informations), back_inserter(information_cache_));
    }
#endif
}

} // namespace mmotd::information
