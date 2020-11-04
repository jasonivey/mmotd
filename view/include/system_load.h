// vim: awa:sts=5:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "view/include/computer_information_provider.h"

namespace mmotd {

class SystemLoad : public ComputerInformationProvider {
public:
    SystemLoad() = default;
    virtual ~SystemLoad() = default;
    SystemLoad(const SystemLoad& other) = default;
    SystemLoad(SystemLoad&& other) noexcept = default;
    SystemLoad& operator=(const SystemLoad& other) = default;
    SystemLoad& operator=(SystemLoad&& other) noexcept = default;

protected:
    std::optional<std::string> QueryInformation() override;
    std::string GetName() const override;
};

}
