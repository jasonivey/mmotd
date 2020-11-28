// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace mmotd {

using ComputerValue = std::tuple<std::string, std::string>;
using ComputerValues = std::vector<ComputerValue>;

class InformationProvider {
public:
    InformationProvider() = default;
    virtual ~InformationProvider() = default;
    InformationProvider(const InformationProvider &other) = default;
    InformationProvider(InformationProvider &&other) noexcept = default;
    InformationProvider &operator=(const InformationProvider &other) = default;
    InformationProvider &operator=(InformationProvider &&other) noexcept = default;

    virtual std::string GetName() const = 0;
    virtual bool QueryInformation() = 0;
    virtual std::optional<ComputerValues> GetInformation() const = 0;
};

} // namespace mmotd
