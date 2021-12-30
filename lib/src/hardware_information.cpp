// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "lib/include/hardware_information.h"
#include "lib/include/platform/hardware_information.h"
#include "lib/include/platform/hardware_temperature.h"

#include <optional>
#include <tuple>
#include <vector>

using namespace std;
using mmotd::platform::EndianType;

bool gLinkHardwareInformation = false;

namespace mmotd::information {

static const bool hardware_information_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::HardwareInformation>(); });

void HardwareInformation::FindInformation() {
    auto details = mmotd::platform::GetHardwareInformationDetails();
    if (!details.empty()) {
        CreateInformationObjects(details);
    }
}

void HardwareInformation::CreateInformationObjects(const mmotd::platform::HardwareDetails &details) {
    auto machine_type = GetInfoTemplate(InformationId::ID_HARDWARE_MACHINE_TYPE);
    machine_type.SetValueArgs(details.machine_type);
    AddInformation(machine_type);

    auto machine_model = GetInfoTemplate(InformationId::ID_HARDWARE_MACHINE_MODEL);
    machine_model.SetValueArgs(details.machine_model);
    AddInformation(machine_model);

    auto cpu_core_count = GetInfoTemplate(InformationId::ID_HARDWARE_CPU_CORE_COUNT);
    cpu_core_count.SetValueArgs(details.cpu_core_count);
    AddInformation(cpu_core_count);

    auto cpu_name = GetInfoTemplate(InformationId::ID_HARDWARE_CPU_NAME);
    cpu_name.SetValueArgs(details.cpu_name);
    AddInformation(cpu_name);

    auto byte_order = GetInfoTemplate(InformationId::ID_HARDWARE_CPU_BYTE_ORDER);
    byte_order.SetValueArgs(to_string(details.byte_order));
    AddInformation(byte_order);

    auto gpu_name = GetInfoTemplate(InformationId::ID_HARDWARE_GPU_MODEL_NAME);
    gpu_name.SetValueArgs(details.gpu_name);
    AddInformation(gpu_name);

    auto monitor_name = GetInfoTemplate(InformationId::ID_HARDWARE_MONITOR_NAME);
    monitor_name.SetValueArgs(details.monitor_name);
    AddInformation(monitor_name);

    auto monitor_resolution = GetInfoTemplate(InformationId::ID_HARDWARE_MONITOR_RESOLUTION);
    monitor_resolution.SetValueArgs(details.monitor_resolution);
    AddInformation(monitor_resolution);

    auto cpu_temperature = GetInfoTemplate(InformationId::ID_HARDWARE_CPU_TEMPERATURE);
    cpu_temperature.SetValueArgs(details.cpu_temperature.to_string());
    AddInformation(cpu_temperature);

    auto gpu_temperature = GetInfoTemplate(InformationId::ID_HARDWARE_GPU_TEMPERATURE);
    gpu_temperature.SetValueArgs(details.gpu_temperature.to_string());
    AddInformation(gpu_temperature);
}

} // namespace mmotd::information
