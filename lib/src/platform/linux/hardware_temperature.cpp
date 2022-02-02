// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__linux__)
#include "common/include/iostream_error.h"
#include "common/include/logging.h"
#include "lib/include/platform/hardware_temperature.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <system_error>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

namespace fs = std::filesystem;
using namespace std;
using namespace std::string_literals;

namespace {

class Acpi {
public:
    // explicit Acpi(string parent_directory = "thermal_zone"s);
    Acpi();
    ~Acpi() = default;

    optional<double> GetCpuTemperature();

private:
    bool DiscoverCpuTemperatureFilename();

    fs::path temperature_path_;
    ifstream input_stream_;
};

// Acpi::Acpi(string parent_directory) {
Acpi::Acpi() {
    static constexpr string_view DEFAULT_PATH = "/sys/class/thermal/thermal_zone0/temp";

    if (!DiscoverCpuTemperatureFilename() || empty(temperature_path_)) {
        LOG_ERROR("unable to find cpu temperature file -- running a virtual machine? (check {})", quoted(DEFAULT_PATH));
        return;
    }

    input_stream_.exceptions(std::ifstream::goodbit);
    input_stream_.open(temperature_path_, ios_base::in);
    if (!input_stream_.is_open() || input_stream_.fail() || input_stream_.bad()) {
        auto input_stream_err = mmotd::error::ios_flags::to_string(input_stream_);
        LOG_ERROR("unable to open {}, {}", temperature_path_, input_stream_err);
        input_stream_.clear();
        return;
    }
}

bool Acpi::DiscoverCpuTemperatureFilename() {
    static constexpr string_view ROOT_DIR = "/sys/class/thermal/";
    static constexpr string_view THERMAL_ZONE_FILENAME = "temp";
    static constexpr string_view THERMAL_ZONE_PATTERN = R"(thermal_zone[\d]+)";

    auto root_dir_path = fs::path{ROOT_DIR};
    auto ec = error_code{};
    if (!fs::exists(root_dir_path, ec) || ec) {
        LOG_ERROR("virtual path {} does not exist, most likely a virtual machine", quoted(ROOT_DIR));
        return false;
    }

    auto dir_iter = fs::directory_iterator{root_dir_path,
                                           fs::directory_options::follow_directory_symlink |
                                               fs::directory_options::skip_permission_denied,
                                           ec};
    if (ec) {
        LOG_ERROR("unable to list directory {}, {}", quoted(ROOT_DIR), ec);
        return false;
    }

    auto thermal_zone_regex_constants =
        regex_constants::ECMAScript | regex_constants::nosubs | regex_constants::optimize;
    auto thermal_zone_regex = regex(string{THERMAL_ZONE_PATTERN}, thermal_zone_regex_constants);

    auto thermal_zone_paths = vector<fs::path>{};
    for (const auto &dir_entry : dir_iter) {
        const auto &dir_entry_path = dir_entry.path();
        if (!dir_entry_path.has_filename()) {
            LOG_VERBOSE("path {} does not have a filename component", quoted(dir_entry_path.string()));
            continue;
        }
        auto file_name = dir_entry_path.filename().string();
        if (!regex_match(file_name, thermal_zone_regex)) {
            LOG_VERBOSE("path {} is not a thermal zone", quoted(dir_entry_path.string()));
            continue;
        }
        LOG_INFO("found thermal zone path: {}", quoted(dir_entry_path.string()));
        thermal_zone_paths.push_back(dir_entry_path);
    }

    temperature_path_.clear();
    for (const auto &thermal_zone_dir : thermal_zone_paths) {
        temperature_path_ = thermal_zone_dir / THERMAL_ZONE_FILENAME;
        if (!fs::exists(temperature_path_, ec) || ec) {
            temperature_path_.clear();
        } else {
            return true;
        }
    }
    return false;
}

optional<double> Acpi::GetCpuTemperature() {
    if (!input_stream_.is_open()) {
        return nullopt;
    }

    auto lines = vector<string>{};
    for (string line; getline(input_stream_, line);) {
        if (input_stream_.fail() || input_stream_.bad()) {
            auto input_stream_err = mmotd::error::ios_flags::to_string(input_stream_);
            LOG_ERROR("error reading {}, {}", temperature_path_, input_stream_err);
            return nullopt;
        }
        lines.push_back(line);
    }
    if (empty(lines)) {
        LOG_ERROR("nothing read from temperature file {}", temperature_path_);
        return nullopt;
    }
    double cpu_temperature = 0.0;
    sscanf(data(lines.front()), "%lf", &cpu_temperature);
    return {cpu_temperature / 1000.0};
}

} // namespace

namespace mmotd::platform {

Temperature GetCpuTemperature() {
    auto acpi = Acpi{};
    auto cpu_temperature = acpi.GetCpuTemperature();
    if (!cpu_temperature) {
        return Temperature{};
    } else {
        return Temperature{*cpu_temperature, Temperature::Units::Celsius};
    }
}

Temperature GetGpuTemperature() {
    return Temperature{};
}

} // namespace mmotd::platform
#endif
