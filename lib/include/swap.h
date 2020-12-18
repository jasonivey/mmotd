// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "lib/include/information_provider.h"

#include <cstdint>
#include <optional>
#include <string>

namespace mmotd {

class Swap : public InformationProvider {
public:
    Swap() = default;
    virtual ~Swap() = default;

    std::string GetName() const override { return std::string{"swap information"}; }
    bool QueryInformation() override;
    std::optional<mmotd::ComputerValues> GetInformation() const override;

private:
    bool GetSwapUsage();

    mmotd::ComputerValues details_;
};

} // namespace mmotd
