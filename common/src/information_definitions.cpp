// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/assertion/include/exception.h"
#include "common/include/information_definitions.h"
#include "common/include/logging.h"

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <string>
#include <utility>

#include <fmt/format.h>
#include <fmt/ostream.h>

using fmt::format;
using namespace std;

namespace mmotd::information {

InformationDefinitions::InformationDefinitions(CategoryIds categories, Informations informations) :
    categories_(std::move(categories)),
    informations_(std::move(informations)) {}

const InformationDefinitions &InformationDefinitions::Instance() {
    static const auto information_definitions = InformationDefinitions{CreateCategoryIds(), CreateInformations()};
    return information_definitions;
}

Information InformationDefinitions::GetInformationDefinition(InformationId id) const {
    auto i = find_if(begin(informations_), end(informations_), [id](const auto &info) { return info.GetId() == id; });
    if (i == end(informations_)) {
        THROW_RUNTIME_ERROR("unable to find information id={}", id);
    }
    return *i;
}

} // namespace mmotd::information
