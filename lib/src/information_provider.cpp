// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/throw.h"
#include "common/include/information.h"
#include "common/include/information_decls.h"
#include "common/include/information_definitions.h"
#include "common/include/logging.h"
#include "lib/include/information_provider.h"

#include <algorithm>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <boost/exception/diagnostic_information.hpp>
#include <fmt/color.h>
#include <fmt/format.h>

using namespace std;
using namespace std::string_literals;

namespace mmotd::information {

InformationProvider::InformationProvider() = default;

InformationProvider::~InformationProvider() = default;

const Informations &InformationProvider::GetInformations() const {
    return informations_;
}

void InformationProvider::LookupInformation() {
    auto exception_message = string{};

    try {
        FindInformation();
    } catch (boost::exception &ex) {
        exception_message = mmotd::assertion::GetBoostExceptionMessage(ex);
    } catch (const std::exception &ex) {
        exception_message = mmotd::assertion::GetStdExceptionMessage(ex);
    } catch (...) { exception_message = mmotd::assertion::GetUnknownExceptionMessage(); }

    if (!empty(exception_message)) {
        LOG_ERROR("{}", exception_message);
    }
}

void InformationProvider::AddInformation(Information information) {
    if (informations_.contains(information.GetId())) {
        informations_[information.GetId()].push_back(move(information));
    } else {
        informations_.emplace(information.GetId(), vector<Information>{information});
    }
}

Information InformationProvider::GetInfoTemplate(InformationId id) const {
    return InformationDefinitions::Instance().GetInformationDefinition(id);
}

std::string InformationProvider::GetUnknownProperty() const {
    static constexpr auto style = fmt::text_style(fmt::emphasis::bold) | fmt::fg(fmt::terminal_color::bright_yellow);
    static const auto UNKNOWN_STR = fmt::format(style, FMT_STRING("[unknown]"));
    return UNKNOWN_STR;
}

} // namespace mmotd::information
