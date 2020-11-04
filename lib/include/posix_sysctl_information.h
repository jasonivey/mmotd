// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>

class PosixSysctlInformation {
    friend std::ostream &operator<<(std::ostream &out, const PosixSysctlInformation &sysctl_info);
public:
    PosixSysctlInformation () = default;

    std::string to_string() const;
    bool TryDiscovery();

private:
};
