// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/platform/hardware_temperature.h"
#if defined(__APPLE__)
#include "common/assertion/include/precondition.h"
#include "common/include/logging.h"
#include "common/include/posix_error.h"
#include "lib/include/platform/hardware_information.h"
#include "lib/include/system_details.h"

#include <array>
#include <cstddef>
#include <iterator>
#include <string_view>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <sys/sysctl.h>
#include <unistd.h>

using fmt::format;
using namespace std;
using namespace mmotd::system;

namespace {

constexpr const int SYSTEM_PROFILER_TIMEOUT = 2; // anything more than 2 seconds and this application is useless!
optional<string> RunSystemProfiler(string system_profiler_type);

struct MachineInformation {
    MachineInformation() = default;

    static optional<MachineInformation> QueryMachineInformation();

    bool empty() const noexcept;

    string machine_type;
    string machine_model;
};

struct GpuInformation {
    GpuInformation() = default;

    static optional<GpuInformation> QueryGpuInformation();

    void AddGraphicsModelName(string model);
    string GetGraphicsModelName() const;

    void AddDisplayName(string display_name);
    string GetDisplayName() const;

    void AddResolution(string resolution);
    string GetResolution() const;

    bool empty() const noexcept;
    // string to_string() const;

private:
    vector<string> graphics_model_names;
    vector<string> display_names;
    vector<string> resolutions;
};

bool MachineInformation::empty() const noexcept {
    return std::empty(machine_type) && std::empty(machine_model);
}

optional<string> GetJsonStringFromPointer(const nlohmann::json &root, std::string_view pointer_str) {
    using nlohmann::json, nlohmann::json_pointer;

    auto json_pointer_obj = json_pointer<json>(data(pointer_str));
    auto str_value = root.value(json_pointer_obj, "");

    if (!str_value.empty()) {
        return {str_value};
    } else {
        return nullopt;
    }
}

optional<MachineInformation> MachineInformation::QueryMachineInformation() {
    using nlohmann::json, nlohmann::json_pointer;

    static const char *const system_profiler_type = "SPHardwareDataType";
    auto output = RunSystemProfiler(system_profiler_type);
    if (!output) {
        return nullopt;
    }

    auto root = json::parse(*output, nullptr, false, true);
    if (root.is_null()) {
        LOG_ERROR("json parse failed on the output of 'system_profiler {}', output: {}", system_profiler_type, *output);
        return nullopt;
    }

    auto hardware_data_root = root.find("SPHardwareDataType");
    if (hardware_data_root == std::end(root) || !hardware_data_root->is_array()) {
        LOG_VERBOSE("unable to find hardware details using: 'system_profiler {}' returned:\n{}",
                    system_profiler_type,
                    *output);
        return nullopt;
    }

    auto machine_details = MachineInformation{};
    for (auto i = size_t{0}; i != hardware_data_root->size(); ++i) {
        auto machine_type_query = format(FMT_STRING("/SPHardwareDataType/{}/machine_name"), i);
        auto machine_type = GetJsonStringFromPointer(root, machine_type_query);
        machine_details.machine_type = machine_type.value_or(string{});

        auto machine_model_query = format(FMT_STRING("/SPHardwareDataType/{}/machine_model"), i);
        auto machine_model = GetJsonStringFromPointer(root, machine_model_query);
        machine_details.machine_model = machine_model.value_or(string{});
    }

    return {machine_details};
}

optional<GpuInformation> GpuInformation::QueryGpuInformation() {
    using nlohmann::json, nlohmann::json_pointer;

    static const char *const system_profiler_type = "SPDisplaysDataType";
    auto output = RunSystemProfiler(system_profiler_type);
    if (!output) {
        return nullopt;
    }

    auto root = json::parse(*output, nullptr, false, true);
    if (root.is_null()) {
        LOG_ERROR("json parse failed using the output of 'system_profiler {}'", system_profiler_type);
        return nullopt;
    }

    auto displays_data_iter = root.find("SPDisplaysDataType");
    if (displays_data_iter == std::end(root) || !displays_data_iter->is_array()) {
        LOG_VERBOSE("unable to locate object SPDisplaysDataType using output of 'system_profiler':\n{}", *output);
        return nullopt;
    }

    const auto &displays_data_root = *displays_data_iter;
    auto gpu_information = GpuInformation{};
    for (auto i = size_t{0}; i != std::size(displays_data_root); ++i) {
        auto graphics_model_query = format(FMT_STRING("/SPDisplaysDataType/{}/sppci_model"), i);
        auto graphics_model = GetJsonStringFromPointer(root, graphics_model_query);
        gpu_information.AddGraphicsModelName(graphics_model.value_or(string{}));

        auto index = static_cast<json::difference_type>(i);
        auto displays_drivers_iter = displays_data_root[index].find("spdisplays_ndrvs");
        if (displays_drivers_iter == std::end(displays_data_root[index]) || !displays_drivers_iter->is_array()) {
            continue;
        }

        const auto &displays_drivers = *displays_drivers_iter;
        for (auto j = size_t{0}; j < std::size(displays_drivers); ++j) {
            auto display_name_query = format(FMT_STRING("/SPDisplaysDataType/{}/spdisplays_ndrvs/{}/_name"), i, j);
            auto display_name = GetJsonStringFromPointer(root, display_name_query);
            gpu_information.AddDisplayName(display_name.value_or(string{}));

            auto resolution_query =
                format(FMT_STRING("/SPDisplaysDataType/{}/spdisplays_ndrvs/{}/_spdisplays_resolution"), i, j);
            auto resolution = GetJsonStringFromPointer(root, resolution_query);
            gpu_information.AddResolution(resolution.value_or(string{}));
        }
    }
    return {gpu_information};
}

bool GpuInformation::empty() const noexcept {
    return std::empty(graphics_model_names) && std::empty(display_names) && std::empty(resolutions);
}

void GpuInformation::AddGraphicsModelName(string model) {
    if (!std::empty(model)) {
        graphics_model_names.push_back(model);
    }
}
string GpuInformation::GetGraphicsModelName() const {
    return format(FMT_STRING("{}"), fmt::join(graphics_model_names, " / "));
}

void GpuInformation::AddDisplayName(string display_name) {
    if (!std::empty(display_name)) {
        display_names.push_back(display_name);
    }
}
string GpuInformation::GetDisplayName() const {
    return format(FMT_STRING("{}"), fmt::join(display_names, " / "));
}

void GpuInformation::AddResolution(string resolution) {
    if (!std::empty(resolution)) {
        resolutions.push_back(boost::replace_first_copy(resolution, " x ", "x"));
    }
}
string GpuInformation::GetResolution() const {
    return format(FMT_STRING("{}"), fmt::join(resolutions, " / "));
}

optional<string> RunSystemProfiler(string system_profiler_type) {
    namespace bp = boost::process;
    namespace io = boost::asio;

    auto exit_code = std::future<int>{};
    auto data = std::future<std::string>{};
    auto io_service = io::io_service{};
    auto command_str = format(FMT_STRING("/usr/sbin/system_profiler {} -json -timeout {} -detailLevel full"),
                              system_profiler_type,
                              SYSTEM_PROFILER_TIMEOUT);
    auto child_process = bp::child(command_str,
                                   bp::std_in.close(),
                                   bp::std_out > data,
                                   bp::std_err > bp::null,
                                   bp::on_exit = exit_code,
                                   io_service);
    io_service.run();
    auto result = exit_code.get();
    auto output = data.get();
    if (result != 0) {
        LOG_ERROR("system_profiler returned error {}, output:\n{}", result, output);
        return nullopt;
    }
    if (empty(output)) {
        return nullopt;
    } else {
        return {output};
    }
}

optional<int32_t> GetCpuCount() {
    auto cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpu_count == -1) {
        LOG_ERROR("sysconf returned -1 calling get processor count _SC_NPROCESSORS_ONLN");
        return nullopt;
    } else {
        LOG_INFO("sysconf for _SC_NPROCESSORS_ONLN returned {} processors", cpu_count);
        return make_optional(cpu_count);
    }
}

mmotd::platform::EndianType GetByteOrder() {
    using mmotd::platform::EndianType;
    uint32_t byte_order = 0;
    auto buffer_size = size_t{4};
    if (sysctlbyname("hw.byteorder", &byte_order, &buffer_size, nullptr, 0) == -1) {
        auto error_str = mmotd::error::posix_error::to_string();
        LOG_ERROR("error calling sysctlbyname with hw.byteorder, details: {}", error_str);
        return mmotd::platform::EndianType::unknown;
    }
    static constexpr uint32_t BIG_ENDIAN_REPR = 4321;
    static constexpr uint32_t LITTLE_ENDIAN_REPR = 1234;
    if (byte_order == BIG_ENDIAN_REPR) {
        return EndianType::big;
    } else if (byte_order == LITTLE_ENDIAN_REPR) {
        return EndianType::little;
    } else {
        return EndianType::unknown;
    }
}

string GetCpuName() {
    static constexpr size_t BUFFER_SIZE = 512;
    auto brand_name = array<char, BUFFER_SIZE>{};
    memset(data(brand_name), 0, BUFFER_SIZE);
    auto buffer_size = BUFFER_SIZE;
    if (sysctlbyname("machdep.cpu.brand_string", data(brand_name), &buffer_size, nullptr, 0) == -1) {
        auto error_str = mmotd::error::posix_error::to_string();
        LOG_ERROR("error calling sysctlnametomib with machdep.cpu.brand_string, details: {}", error_str);
        return string{};
    }
    return string(data(brand_name));
}

} // namespace

namespace mmotd::platform {

HardwareDetails GetHardwareInformationDetails() {
    auto details = HardwareDetails{};
    details.cpu_core_count = GetCpuCount().value_or(0);
    details.cpu_name = GetCpuName();
    details.byte_order = GetByteOrder();
    details.cpu_temperature = GetCpuTemperature();
    details.gpu_temperature = GetGpuTemperature();

    auto gpu_information_holder = GpuInformation::QueryGpuInformation();
    if (!gpu_information_holder || gpu_information_holder.value().empty()) {
        LOG_WARNING("gpu information was either null or empty");
    } else {
        const auto &gpu_info = *gpu_information_holder;
        details.gpu_name = gpu_info.GetGraphicsModelName();
        details.monitor_name = gpu_info.GetDisplayName();
        details.monitor_resolution = gpu_info.GetResolution();
    }

    auto machine_information_holder = MachineInformation::QueryMachineInformation();
    if (!machine_information_holder || machine_information_holder.value().empty()) {
        LOG_WARNING("machine information was either null or empty");
    } else {
        const auto &machine_info = *machine_information_holder;
        details.machine_type = machine_info.machine_type;
        details.machine_model = machine_info.machine_model;
    }

    return details;
}

} // namespace mmotd::platform
#endif
