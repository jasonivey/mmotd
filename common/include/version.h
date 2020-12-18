// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <memory>
#include <string>
#include <string_view>

namespace mmotd::semver {
struct version;
} // namespace mmotd::semver

namespace mmotd::version {

class Version {
public:
    static const Version &Instance();

    std::string to_string() const;
    std::int32_t GetMajor() const;
    std::int32_t GetMinor() const;
    std::int32_t GetPatch() const;
    std::string GetPrerelease() const;
    std::string GetBuildNumber() const;

private:
    Version(std::string_view version_str);

    std::unique_ptr<mmotd::semver::version> version_;
};

} // namespace mmotd::version
