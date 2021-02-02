// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"
#include "lib/include/information_provider.h"

#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace mmotd::platform {

using SystemInformationDetail = std::tuple<std::string, std::string>;
using SystemInformationDetails = std::vector<SystemInformationDetail>;

SystemInformationDetails GetSystemInformationDetails();

} // namespace mmotd::platform

namespace mmotd::information {

class SystemInformation : public InformationProvider {
public:
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_VIRTUAL_DESTRUCTOR(SystemInformation);

protected:
    bool FindInformation() override;

private:
    void CreateInformationObjects(const mmotd::platform::SystemInformationDetails &details);
};

} // namespace mmotd::information
