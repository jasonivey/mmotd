// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/information.h"
#include "common/include/information_decls.h"
#include "common/include/information_definitions.h"
#include "lib/include/information_provider.h"

#include <algorithm>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include <boost/exception/diagnostic_information.hpp>
#include <fmt/format.h>

using namespace std;

namespace mmotd::information {

InformationProvider::InformationProvider() = default;

InformationProvider::~InformationProvider() = default;

const std::vector<Information> &InformationProvider::GetInformations() const {
    return informations_;
}

void InformationProvider::LookupInformation() {
    try {
        FindInformation();
    } catch (boost::exception &ex) {
        auto diag = boost::diagnostic_information(ex);
        LOG_ERROR("caught boost::exception in LookupInformation: {}", diag);
    } catch (const std::exception &ex) {
        auto diag = boost::diagnostic_information(ex);
        LOG_ERROR("caught std::exception in LookupInformation: {}", empty(diag) ? ex.what() : data(diag));
    }
}

void InformationProvider::AddInformation(Information information) {
    informations_.push_back(information);
}

Information InformationProvider::GetInfoTemplate(InformationId id) const {
    return InformationDefinitions::Instance().GetInformationDefinition(id);
}

} // namespace mmotd::information
