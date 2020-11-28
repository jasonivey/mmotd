// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "lib/include/information_provider.h"

#include <cstdint>
#include <optional>
#include <string>

namespace mmotd {

class ExternalNetwork : public InformationProvider {
public:
    ExternalNetwork() = default;

    std::string GetName() const override { return std::string{"external network"}; }
    bool QueryInformation() override;
    std::optional<mmotd::ComputerValues> GetInformation() const override;

private:
    bool RequestExternalIpAddress();
    bool ParseJsonResponse(const std::string &response);

    mmotd::ComputerValues details_;
};

} // namespace mmotd
