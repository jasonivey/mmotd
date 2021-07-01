// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <string>

namespace mmotd::platform {

struct SystemDetails {
    std::string host_name;
    std::string computer_name;
    std::string kernel_version;
    std::string kernel_release;
    std::string kernel_type;
    std::string architecture_type;
    std::string platform_version;
    std::string platform_name;

    bool empty() const noexcept;
};

SystemDetails GetSystemInformationDetails();

} // namespace mmotd::platform
