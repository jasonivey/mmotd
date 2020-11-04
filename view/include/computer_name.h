// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "view/include/computer_information_provider.h"

namespace mmotd {

class ComputerName : public ComputerInformationProvider {
public:
    ComputerName() = default;
    virtual ~ComputerName() = default;
    ComputerName(const ComputerName& other) = default;
    ComputerName(ComputerName&& other) noexcept = default;
    ComputerName& operator=(const ComputerName& other) = default;
    ComputerName& operator=(ComputerName&& other) noexcept = default;

protected:
    std::optional<std::string> QueryInformation() override;
    std::string GetName() const override;
};

}
