// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "view/include/computer_information_provider.h"

namespace mmotd {

class HostName : public ComputerInformationProvider {
public:
    HostName() = default;
    virtual ~HostName() = default;
    HostName(const HostName &other) = default;
    HostName(HostName &&other) noexcept = default;
    HostName &operator=(const HostName &other) = default;
    HostName &operator=(HostName &&other) noexcept = default;

protected:
    std::optional<std::string> QueryInformation() override;
    std::string GetName() const override;
};

} // namespace mmotd
