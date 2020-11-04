// vim: awa:sts=5:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "view/include/computer_information_provider.h"

namespace mmotd {

class SubHeader : public ComputerInformationProvider {
public:
    SubHeader() = default;
    virtual ~SubHeader() = default;
    SubHeader(const SubHeader& other) = default;
    SubHeader(SubHeader&& other) noexcept = default;
    SubHeader& operator=(const SubHeader& other) = default;
    SubHeader& operator=(SubHeader&& other) noexcept = default;

protected:
    std::optional<std::string> QueryInformation() override;
    std::string GetName() const override;
};

}
