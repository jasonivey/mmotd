// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"
#include "common/include/information_definitions.h"
#include "lib/include/information_provider.h"

#include <cstdint>
#include <optional>
#include <string>

namespace mmotd::information {

class LoadAverage : public InformationProvider {
public:
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_VIRTUAL_DESTRUCTOR(LoadAverage);

protected:
    void FindInformation() override;
};

} // namespace mmotd::information
