// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <cstdint>
#include <limits>
#include <string>

struct passwd;

namespace mmotd::core {

struct UserInformation {
    std::string username;
    std::uint32_t user_id = std::numeric_limits<std::uint32_t>::max();
    std::uint32_t group_id = std::numeric_limits<std::uint32_t>::max();
    std::string full_name;
    std::string home_directory;
    std::string shell;

    bool empty() const { return user_id == std::numeric_limits<std::uint32_t>::max() && username.empty(); }

    static UserInformation from_passwd(const passwd &pw);
};

const UserInformation &GetUserInformation();

} // namespace mmotd::core
