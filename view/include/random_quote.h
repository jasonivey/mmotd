// vim: awa:sts=5:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "view/include/computer_information_provider.h"

namespace mmotd {

class RandomQuote : public ComputerInformationProvider {
public:
    RandomQuote() = default;
    virtual ~RandomQuote() = default;
    RandomQuote(const RandomQuote& other) = default;
    RandomQuote(RandomQuote&& other) noexcept = default;
    RandomQuote& operator=(const RandomQuote& other) = default;
    RandomQuote& operator=(RandomQuote&& other) noexcept = default;

protected:
    std::optional<std::string> QueryInformation() override;
    std::string GetName() const override;
};

}
