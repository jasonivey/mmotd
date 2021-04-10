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

using namespace std;

namespace mmotd::information {

InformationProvider::InformationProvider() {
}

InformationProvider::~InformationProvider() {
}

const std::vector<Information> &InformationProvider::GetInformations() const {
    return informations_;
}

bool InformationProvider::LookupInformation() {
    return FindInformation();
}

void InformationProvider::AddInformation(Information information) {
    informations_.push_back(information);
}

Information InformationProvider::GetInfoTemplate(InformationId id) const {
    return InformationDefinitions::Instance().GetInformationDefinition(id);
}

} // namespace mmotd::information
