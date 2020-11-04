// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "view/include/computer_information_provider.h"

namespace mmotd {

class Greeting : public ComputerInformationProvider {
public:
    Greeting() = default;
    virtual ~Greeting() = default;
    Greeting(const Greeting& other) = default;
    Greeting(Greeting&& other) noexcept = default;
    Greeting& operator=(const Greeting& other) = default;
    Greeting& operator=(Greeting&& other) noexcept = default;

protected:
    std::optional<std::string> QueryInformation() override;
    std::string GetName() const override;
};

}
