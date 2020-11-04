// vim: awa:sts=5:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "view/include/computer_information_provider.h"

namespace mmotd {

class UsersCount : public ComputerInformationProvider {
public:
    UsersCount() = default;
    virtual ~UsersCount() = default;
    UsersCount(const UsersCount& other) = default;
    UsersCount(UsersCount&& other) noexcept = default;
    UsersCount& operator=(const UsersCount& other) = default;
    UsersCount& operator=(UsersCount&& other) noexcept = default;

protected:
    std::optional<std::string> QueryInformation() override;
    std::string GetName() const override;
};

}
