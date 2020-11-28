// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "lib/include/information_provider.h"

#include <cstdint>
#include <ctime>
#include <optional>
#include <string>

namespace mmotd {

class LastLog : public InformationProvider {
public:
    LastLog() = default;
    ~LastLog() = default;

    std::string GetName() const override { return std::string{"last log"}; }
    bool QueryInformation() override;
    std::optional<mmotd::ComputerValues> GetInformation() const override;

private:
    //bool GetNextRecord(UserLoginLogoutTransaction &transaction);
    bool GetLastLoginRecord();
    std::optional<std::tuple<std::string, std::uint32_t>> GetUsername();

    mmotd::ComputerValues last_login_details_;
};

} // namespace mmotd
