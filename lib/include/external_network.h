// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"
#include "common/include/information_definitions.h"
#include "lib/include/information_provider.h"

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

namespace mmotd::information {

class ExternalNetwork : public InformationProvider {
public:
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_VIRTUAL_DESTRUCTOR(ExternalNetwork);

protected:
    void FindInformation() override;

private:
    std::pair<std::string, std::string> GetRequestUrl() const;

    void ParseJsonResponse(const std::string &response);
};

} // namespace mmotd::information
