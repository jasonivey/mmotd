// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "lib/include/information_provider.h"

#include <cstdint>
#include <optional>
#include <string>

namespace mmotd {

class Processes : public InformationProvider {
public:
    Processes() = default;

    std::string GetName() const override { return std::string{"processes"}; }
    bool QueryInformation() override;
    std::optional<mmotd::ComputerValues> GetInformation() const override;

private:
    bool GetProcessCount();

    mmotd::ComputerValues details_;
};

} // namespace mmotd
