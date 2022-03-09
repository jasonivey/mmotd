// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__linux__)
#include "common/include/iostream_error.h"
#include "common/include/logging.h"
#include "common/include/posix_error.h"
#include "common/include/system_command.h"
#include "lib/include/hardware_information.h"
#include "lib/include/platform/hardware_information.h"

#include <charconv>
#include <filesystem>
#include <fstream>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using fmt::format;
using namespace std;
using json = nlohmann::json;
using mmotd::platform::EndianType, mmotd::platform::from_endian_string;

namespace {

struct LscpuNameValue {
    string name;
    string value;
};

struct LscpuContainer {
    vector<LscpuNameValue> name_values;
    optional<string> GetData(const string &field_name) const;
};

optional<string> LscpuContainer::GetData(const string &field_name) const {
    auto i = find_if(begin(name_values), end(name_values), [&field_name](const auto &name_value) {
        return boost::iequals(name_value.name, field_name);
    });
    if (i == end(name_values)) {
        LOG_ERROR("unable to find field '{}' within lscpu name/values", field_name);
        return nullopt;
    }
    return make_optional((*i).value);
}

void from_json(const json &root, LscpuNameValue &lscpu_data) {
    root.at("field").get_to(lscpu_data.name);
    root.at("data").get_to(lscpu_data.value);
}

void from_json(const json &root, LscpuContainer &lscpu_ontainer) {
    root.at("lscpu").get_to(lscpu_ontainer.name_values);
}

tuple<string, int32_t, EndianType> ParseLscpuOutput(optional<string> output_holder) {
    // ensure input is valid
    if (!output_holder) {
        return tuple<string, int32_t, EndianType>{};
    }
    auto output = *output_holder;

    // use nlohmann::json to parse the input
    auto root = json::parse(output, nullptr, false, true);
    if (root.is_null()) {
        LOG_ERROR("parsing output of 'lscpu' into json, output: '{}'", output);
        return tuple<string, int32_t, EndianType>{};
    }

    // serialize the nlohmann::json data to an actual data structure
    auto lscpu = LscpuContainer{};
    from_json(root, lscpu);

    // query the data structure for byte order
    auto byte_order_holder = lscpu.GetData("Byte Order:");
    auto byte_order = EndianType::unknown;
    if (byte_order_holder) {
        byte_order = from_endian_string(*byte_order_holder);
    }

    // query the data structure for cpu count
    auto cpu_count_holder = lscpu.GetData("CPU(s):");
    auto cpu_count = int32_t{1};
    if (cpu_count_holder) {
        auto [p, ec] =
            from_chars(data(*cpu_count_holder), data(*cpu_count_holder) + size(*cpu_count_holder), cpu_count);
        if (ec != std::errc()) {
            LOG_ERROR("unable to convert cpu count ({}) to an integer", *cpu_count_holder);
            cpu_count = int32_t{1};
        }
    }

    // query the data structure for cpu name
    auto cpu_name_holder = lscpu.GetData("Model name:");
    auto cpu_name = cpu_name_holder.value_or(string{});

    return make_tuple(cpu_name, cpu_count, byte_order);
}

tuple<string, int32_t, EndianType> GetCpuInformation() {
    using mmotd::system::command::Run;

    return Run("/usr/bin/lscpu", {"--json"}, ParseLscpuOutput);
}

string GetLineFromFile(const std::filesystem::path &p) {
    auto ec = error_code{};
    if (!fs::exists(p, ec) || ec) {
        LOG_ERROR("unable to locate file for reading: '{}'", p.string());
        return string{};
    }

    auto input = ifstream(p);
    if (!input.is_open()) {
        LOG_ERROR("unable to open file for reading: {}", p.string());
        return string{};
    }

    auto first_line = string{};
    getline(input, first_line);
    return boost::trim_copy(first_line);
}

string GetMachineType() {
    auto machine_type_path = fs::path("/sys/devices/virtual/dmi/id/sys_vendor");
    return GetLineFromFile(machine_type_path);
}

string GetMachineModel() {
    auto product_name_path = fs::path("/sys/devices/virtual/dmi/id/product_name");
    return GetLineFromFile(product_name_path);
}

optional<string> ParseLspciOutput(optional<string> output_holder) {
    // ensure input is valid
    if (!output_holder) {
        return nullopt;
    }

    // split the chunk of input into discrete lines
    auto output = *output_holder;
    auto output_lines = vector<string>{};
    boost::split(output_lines, output, boost::is_any_of("\n"));

    constexpr auto regex_flags = std::regex::ECMAScript | std::regex::icase;
    const auto vga_regex = regex(R"(vga|[23]d)", regex_flags);

    // find which line refers to 'VGA', '3D' or '2D'
    auto i = find_if(begin(output_lines), end(output_lines), [&vga_regex](const auto &output_line) {
        return std::regex_search(output_line, vga_regex, std::regex_constants::match_any);
    });
    if (i == end(output_lines)) {
        LOG_ERROR("unable to find 'VGA', '2D' or '3D' in output of 'lspci', output: '{}'", output);
        return nullopt;
    }

    // split the line on the ":" character
    const auto &vga_line = *i;
    auto vga_line_parts = vector<string>{};
    boost::split(vga_line_parts, vga_line, boost::is_any_of(":"));

    if (size(vga_line_parts) == 1) {
        LOG_ERROR("found 'vga' in 'lspci output but there is no ':' character: '{}'", vga_line);
        return nullopt;
    }

    // trim any whitespace off of the right hand side of the ":" character
    auto gpu_name = boost::trim_copy(vga_line_parts.back());
    LOG_INFO("found gpu model name from 'lspci' output: '{}'", gpu_name);
    return make_optional(gpu_name);
}

fs::path FindLspciBinary() {
    auto ec = error_code{};
    if (auto lspci_path = fs::path{"/usr/bin/lspci"}; fs::exists(lspci_path, ec) && !ec) {
        return lspci_path;
    }
    if (auto lspci_path = fs::path{"/usr/sbin/lspci"}; fs::exists(lspci_path, ec) && !ec) {
        return lspci_path;
    }
    return fs::path{};
}

string GetGraphicsModelName() {
    using mmotd::system::command::Run;
    auto gpu_model_name = Run(FindLspciBinary(), {}, ParseLspciOutput);
    if (!gpu_model_name) {
        LOG_ERROR("lspci did not return the gpu model name");
        return string{};
    }

    LOG_INFO("gpu model name: {}", *gpu_model_name);
    return *gpu_model_name;
}

string GetMonitorName() {
    return string{};
    // return string{"unknown"};
}

string GetMonitorResolution() {
    return string{};
    // return string{"unknown"};
}

} // namespace

namespace mmotd::platform {

HardwareDetails GetHardwareInformationDetails() {
    auto [cpu_name, cpu_count, byte_order] = GetCpuInformation();

    auto details = HardwareDetails{};
    details.machine_type = GetMachineType();
    details.machine_model = GetMachineModel();
    details.cpu_core_count = cpu_count;
    details.cpu_name = cpu_name;
    details.cpu_temperature = GetCpuTemperature();
    details.byte_order = byte_order;
    details.gpu_name = GetGraphicsModelName();
    details.gpu_temperature = GetGpuTemperature();
    details.monitor_name = GetMonitorName();
    details.monitor_resolution = GetMonitorResolution();

    return details;
}

} // namespace mmotd::platform
#endif
