// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "lib/include/information_provider.h"

#include <optional>
#include <string>

namespace mmotd {

class NetworkInfo : public InformationProvider {
public:
    NetworkInfo() = default;
    virtual ~NetworkInfo() = default;

    std::string GetName() const override { return std::string{"network"}; }
    bool QueryInformation() override;
    std::optional<mmotd::ComputerValues> GetInformation() const override;

private:
    ComputerValues network_information_;
};

} // namespace mmotd
