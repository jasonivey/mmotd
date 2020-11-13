// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "view/include/computer_information_provider.h"

namespace mmotd {

class MemoryUsage : public ComputerInformationProvider {
public:
    MemoryUsage() = default;
    virtual ~MemoryUsage() = default;
    MemoryUsage(const MemoryUsage &other) = default;
    MemoryUsage(MemoryUsage &&other) noexcept = default;
    MemoryUsage &operator=(const MemoryUsage &other) = default;
    MemoryUsage &operator=(MemoryUsage &&other) noexcept = default;

protected:
    std::optional<std::string> QueryInformation() override;
    std::string GetName() const override;
};

} // namespace mmotd
