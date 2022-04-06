// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__linux__)
#include "lib/include/hardware_information.h"

#include "common/include/iostream_error.h"
#include "common/include/logging.h"
#include "common/include/posix_error.h"
#include "common/include/system_command.h"
#include "lib/include/platform/hardware_information.h"

#include <bit>
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

using CpuInfo = tuple<string, int32_t, optional<endian>>;

CpuInfo ParseLscpuOutput(optional<string> output_holder) {
    using mmotd::platform::from_endian_string;
    // ensure input is valid
    if (!output_holder) {
        return CpuInfo{};
    }
    auto output = *output_holder;

    // use nlohmann::json to parse the input
    auto root = json::parse(output, nullptr, false, true);
    if (root.is_null()) {
        LOG_ERROR("parsing output of 'lscpu' into json, output: '{}'", output);
        return CpuInfo{};
    }

    // serialize the nlohmann::json data to an actual data structure
    auto lscpu = LscpuContainer{};
    from_json(root, lscpu);

    // query the data structure for byte order
    auto byte_order = optional<endian>{};
    if (auto byte_order_str_holder = lscpu.GetData("Byte Order:"); !byte_order_str_holder) {
        LOG_ERROR("lscpu output does not contain 'Byte Order:'");
    } else {
        if (auto byte_order_holder = from_endian_string(*byte_order_str_holder); !byte_order_holder) {
            LOG_ERROR("unable to parse '{}' as endian", *byte_order_str_holder);
        } else {
            byte_order = byte_order_holder;
        }
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

CpuInfo GetCpuInformation() {
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
