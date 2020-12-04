// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "lib/include/information_provider.h"

#include <cstdint>
#include <optional>
#include <string>

namespace mmotd {

class UsersLoggedIn : public InformationProvider {
public:
    UsersLoggedIn() = default;

    std::string GetName() const override { return std::string{"user session"}; }
    bool QueryInformation() override;
    std::optional<mmotd::ComputerValues> GetInformation() const override;

private:
    bool GetUsersLoggedIn();

    mmotd::ComputerValues details_;
};

} // namespace mmotd
