// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"
#include "lib/include/information_provider.h"

#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace mmotd::platform {

struct HardwareDetails;

} // namespace mmotd::platform

namespace mmotd::information {

class HardwareInformation : public InformationProvider {
public:
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_VIRTUAL_DESTRUCTOR(HardwareInformation);

protected:
    void FindInformation() override;

private:
    void CreateInformationObjects(const mmotd::platform::HardwareDetails &details);
};

} // namespace mmotd::information
