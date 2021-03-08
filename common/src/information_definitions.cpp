// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/information_definitions.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iterator>
#include <stdexcept>
#include <string>
#include <thread>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

namespace mmotd::information {

InformationDefinitions::InformationDefinitions(CategoryIds categories, Informations informations) :
    categories_(categories), informations_(informations) {
}

const InformationDefinitions &InformationDefinitions::Instance() {
    static const auto information_definitions = InformationDefinitions{CreateCategoryIds(), CreateInformations()};
    return information_definitions;
}

Information InformationDefinitions::GetInformationDefinition(InformationId id) const {
    //const auto lock = lock_guard<mutex>(mutex_);
    auto i = find_if(begin(informations_), end(informations_), [id](const auto &info) { return info.GetId() == id; });
    if (i == end(informations_)) {
        auto msg = format("unable to find information id={}", id);
        PLOG_ERROR << msg;
        throw std::runtime_error(msg);
    }
    return *i;
}

} // namespace mmotd::information
