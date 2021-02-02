// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"
#include "common/include/information.h"

#include <array>
#include <initializer_list>
#include <optional>
#include <string>
#include <vector>

namespace mmotd::information {

enum class InformationId : size_t;

class InformationProvider {
public:
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_VIRTUAL_DESTRUCTOR(InformationProvider);

    const std::vector<Information> &GetInformations() const;

    bool LookupInformation();

protected:
    virtual bool FindInformation() = 0;

    void AddInformation(Information information);

    Information GetInfoTemplate(InformationId id) const;

private:
    std::vector<Information> informations_;
};

} // namespace mmotd::information
