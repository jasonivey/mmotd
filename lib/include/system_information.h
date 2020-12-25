// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "lib/include/information_provider.h"

#include <optional>
#include <string>

namespace mmotd {

class SystemInformation : public InformationProvider {
public:
    SystemInformation() = default;
    virtual ~SystemInformation() = default;

    std::string GetName() const override { return std::string{"system information"}; }
    bool QueryInformation() override;
    std::optional<mmotd::ComputerValues> GetInformation() const override;

private:
    mmotd::ComputerValues details_;
};

} // namespace mmotd