// vim: awa:sts=5:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "view/include/computer_information_provider.h"

namespace mmotd {

class PublicIp : public ComputerInformationProvider {
public:
    PublicIp() = default;
    virtual ~PublicIp() = default;
    PublicIp(const PublicIp& other) = default;
    PublicIp(PublicIp&& other) noexcept = default;
    PublicIp& operator=(const PublicIp& other) = default;
    PublicIp& operator=(PublicIp&& other) noexcept = default;

protected:
    std::optional<std::string> QueryInformation() override;
    std::string GetName() const override;
};

}
