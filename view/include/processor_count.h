// vim: awa:sts=5:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "view/include/computer_information_provider.h"

namespace mmotd {

class ProcessorCount : public ComputerInformationProvider {
public:
    ProcessorCount() = default;
    virtual ~ProcessorCount() = default;
    ProcessorCount(const ProcessorCount& other) = default;
    ProcessorCount(ProcessorCount&& other) noexcept = default;
    ProcessorCount& operator=(const ProcessorCount& other) = default;
    ProcessorCount& operator=(ProcessorCount&& other) noexcept = default;

protected:
    std::optional<std::string> QueryInformation() override;
    std::string GetName() const override;
};

}
