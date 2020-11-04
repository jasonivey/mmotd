// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace mmotd {

class ComputerInformationProvider {
public:
    ComputerInformationProvider() = default;
    virtual ~ComputerInformationProvider() = default;
    ComputerInformationProvider(const ComputerInformationProvider& other) = default;
    ComputerInformationProvider(ComputerInformationProvider&& other) noexcept = default;
    ComputerInformationProvider& operator=(const ComputerInformationProvider& other) = default;
    ComputerInformationProvider& operator=(ComputerInformationProvider&& other) noexcept = default;

    std::optional<std::string> GetComputerInformation();
    std::string GetInformationName() const;

protected:
    virtual std::optional<std::string> QueryInformation() = 0;
    virtual std::string GetName() const = 0;
};

}
