// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "view/include/computer_information_provider.h"

namespace mmotd {

class Header : public ComputerInformationProvider {
public:
    Header() = default;
    virtual ~Header() = default;
    Header(const Header& other) = default;
    Header(Header&& other) noexcept = default;
    Header& operator=(const Header& other) = default;
    Header& operator=(Header&& other) noexcept = default;

protected:
    std::optional<std::string> QueryInformation() override;
    std::string GetName() const override;
};

}
