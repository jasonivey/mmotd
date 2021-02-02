// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"
#include "lib/include/information_provider.h"

#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

namespace mmotd::information {

class UsersLoggedIn : public InformationProvider {
public:
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_VIRTUAL_DESTRUCTOR(UsersLoggedIn);

protected:
    bool FindInformation() override;

    using Detail = std::tuple<std::string, std::string>;
    using Details = std::vector<Detail>;
    std::string GetUsersLoggedIn();
};

} // namespace mmotd::information
