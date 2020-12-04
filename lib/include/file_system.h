// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "lib/include/information_provider.h"

#include <cstdint>
#include <ctime>
#include <optional>
#include <string>

namespace mmotd {

class FileSystem : public InformationProvider {
public:
    FileSystem() = default;
    ~FileSystem() = default;

    std::string GetName() const override { return std::string{"file system"}; }
    bool QueryInformation() override;
    std::optional<mmotd::ComputerValues> GetInformation() const override;

private:
    bool GetFileSystemDetails();

    mmotd::ComputerValues file_system_details_;
};

} // namespace mmotd
