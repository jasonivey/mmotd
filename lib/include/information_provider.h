// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"

#include <unordered_map>
#include <vector>

namespace mmotd::information {

enum class InformationId : size_t;
class Information;
using Informations = std::unordered_map<InformationId, std::vector<Information>>;

class InformationProvider {
public:
    InformationProvider();
    virtual ~InformationProvider();
    NO_CONSTRUCTOR_DEFAULT_COPY_MOVE_OPERATORS_NO_DESTRUCTOR(InformationProvider);

    const Informations &GetInformations() const;

    void LookupInformation();

protected:
    virtual void FindInformation() = 0;

    void AddInformation(Information information);

    Information GetInfoTemplate(InformationId id) const;

private:
    Informations informations_;
};

} // namespace mmotd::information
