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
    InformationProvider();
    InformationProvider(InformationProvider const &) = default;
    InformationProvider &operator=(InformationProvider const &) = default;
    InformationProvider(InformationProvider &&) = default;
    InformationProvider &operator=(InformationProvider &&) = default;
    virtual ~InformationProvider();

    const std::vector<Information> &GetInformations() const;

    void LookupInformation();

protected:
    virtual void FindInformation() = 0;

    void AddInformation(Information information);

    Information GetInfoTemplate(InformationId id) const;

private:
    std::vector<Information> informations_;
};

} // namespace mmotd::information
