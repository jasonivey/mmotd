// vim: awa:sts=5:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "view/include/computer_information_provider.h"

namespace mmotd {

class ProcessesCount : public ComputerInformationProvider {
public:
    ProcessesCount() = default;
    virtual ~ProcessesCount() = default;
    ProcessesCount(const ProcessesCount &other) = default;
    ProcessesCount(ProcessesCount &&other) noexcept = default;
    ProcessesCount &operator=(const ProcessesCount &other) = default;
    ProcessesCount &operator=(ProcessesCount &&other) noexcept = default;

protected:
    std::optional<std::string> QueryInformation() override;
    std::string GetName() const override;
};

} // namespace mmotd
