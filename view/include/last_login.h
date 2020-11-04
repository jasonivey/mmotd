// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "view/include/computer_information_provider.h"

namespace mmotd {

class LastLogin : public ComputerInformationProvider {
public:
    LastLogin() = default;
    virtual ~LastLogin() = default;
    LastLogin(const LastLogin& other) = default;
    LastLogin(LastLogin&& other) noexcept = default;
    LastLogin& operator=(const LastLogin& other) = default;
    LastLogin& operator=(LastLogin&& other) noexcept = default;

protected:
    std::optional<std::string> QueryInformation() override;
    std::string GetName() const override;
};

}
