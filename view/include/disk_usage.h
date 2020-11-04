// vim: awa:sts=5:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "view/include/computer_information_provider.h"

namespace mmotd {

class DiskUsage : public ComputerInformationProvider {
public:
    DiskUsage() = default;
    virtual ~DiskUsage() = default;
    DiskUsage(const DiskUsage& other) = default;
    DiskUsage(DiskUsage&& other) noexcept = default;
    DiskUsage& operator=(const DiskUsage& other) = default;
    DiskUsage& operator=(DiskUsage&& other) noexcept = default;

protected:
    std::optional<std::string> QueryInformation() override;
    std::string GetName() const override;
};

}
