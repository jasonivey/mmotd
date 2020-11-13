// vim: awa:sts=5:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "view/include/computer_information_provider.h"

namespace mmotd {

class SwapUsage : public ComputerInformationProvider {
public:
    SwapUsage() = default;
    virtual ~SwapUsage() = default;
    SwapUsage(const SwapUsage &other) = default;
    SwapUsage(SwapUsage &&other) noexcept = default;
    SwapUsage &operator=(const SwapUsage &other) = default;
    SwapUsage &operator=(SwapUsage &&other) noexcept = default;

protected:
    std::optional<std::string> QueryInformation() override;
    std::string GetName() const override;
};

} // namespace mmotd
