// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "view/include/computer_information_provider.h"

namespace mmotd {

class ActiveNetworkInterfaces : public ComputerInformationProvider {
public:
    ActiveNetworkInterfaces() = default;
    virtual ~ActiveNetworkInterfaces() = default;
    ActiveNetworkInterfaces(const ActiveNetworkInterfaces& other) = default;
    ActiveNetworkInterfaces(ActiveNetworkInterfaces&& other) noexcept = default;
    ActiveNetworkInterfaces& operator=(const ActiveNetworkInterfaces& other) = default;
    ActiveNetworkInterfaces& operator=(ActiveNetworkInterfaces&& other) noexcept = default;

protected:
    std::optional<std::string> QueryInformation() override;
    std::string GetName() const override;
};

}
