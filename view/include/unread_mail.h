// vim: awa:sts=5:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "view/include/computer_information_provider.h"

namespace mmotd {

class UnreadMail : public ComputerInformationProvider {
public:
    UnreadMail() = default;
    virtual ~UnreadMail() = default;
    UnreadMail(const UnreadMail& other) = default;
    UnreadMail(UnreadMail&& other) noexcept = default;
    UnreadMail& operator=(const UnreadMail& other) = default;
    UnreadMail& operator=(UnreadMail&& other) noexcept = default;

protected:
    std::optional<std::string> QueryInformation() override;
    std::string GetName() const override;
};

}
