// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__APPLE__)
#include "common/assertion/include/assertion.h"
#include "common/include/logging.h"
#include "lib/include/platform/hardware_temperature.h"

#include <array>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include <arpa/inet.h>

#include <IOKit/IOKitLib.h>

using namespace std;

namespace {

constexpr const int KERNEL_INDEX = 2;

constexpr const char READ_BYTES_CMD = 5;
constexpr const char READ_KEYINFO_CMD = 9;

static const char *const DATATYPE_SP78 = "sp78";

struct KeyDataVersion {
    char major = 0;
    char minor = 0;
    char build = 0;
    array<char, 1> reserved = {0};
    uint16_t release = 0;
};

struct PLimitData {
    uint16_t version = 0;
    uint16_t length = 0;
    uint32_t cpu_plimit = 0;
    uint32_t gpu_plimit = 0;
    uint32_t mem_plimit = 0;
};

struct KeyInfo {
    uint32_t data_size = 0;
    uint32_t data_type = 0;
    char data_attributes = 0;
};

using ByteBuffer = array<unsigned char, 32>;

struct KeyData {
    uint32_t key = 0;
    KeyDataVersion version;
    PLimitData plimit_data;
    KeyInfo key_info;
    char result = 0;
    char status = 0;
    char data8 = 0;
    uint32_t data32 = 0;
    ByteBuffer bytes = {0};
};

using KeyBuffer = array<char, 5>;

struct Value {
    KeyBuffer key = {0};
    uint32_t data_size = 0;
    KeyBuffer data_type = {0};
    ByteBuffer bytes = {0};
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
uint32_t _strtoul(const char *str, int size, int base) {
    uint32_t total = 0;
    for (int i = 0; i < size; i++) {
        if (base == 16)
            total += str[i] << (size - 1 - i) * 8;
        else
            total += ((unsigned char)(str[i]) << (size - 1 - i) * 8);
    }
    return total;
}
#pragma GCC diagnostic pop

uint32_t BufferToNum(const KeyBuffer &buffer, int base) {
    uint32_t total = 0;
    auto buf_size = size(buffer) - 1;
    for (size_t i = 0ul; i != buf_size; ++i) {
        if (base == 16) {
            total += buffer[i] << (buf_size - 1 - i) * 8;
        } else {
            total += (static_cast<unsigned char>(buffer[i]) << (buf_size - 1 - i) * 8);
        }
    }
    CHECKS(total == _strtoul(data(buffer), static_cast<int>(buf_size), base), "this better work");
    return total;
}

void _ultostr(KeyBuffer &str, uint32_t val) {
    str[0] = '\0';
    sprintf(data(str),
            "%c%c%c%c",
            static_cast<unsigned int>(val) >> 24,
            static_cast<unsigned int>(val) >> 16,
            static_cast<unsigned int>(val) >> 8,
            static_cast<unsigned int>(val));
}

KeyBuffer NumToBuffer(uint32_t value) {
    auto str = fmt::format(FMT_STRING("{:c}{:c}{:c}{:c}"),
                           static_cast<uint32_t>(value) >> 24,
                           static_cast<uint32_t>(value) >> 16,
                           static_cast<uint32_t>(value) >> 8,
                           static_cast<uint32_t>(value));
    auto buffer = KeyBuffer{};
    copy(begin(str), end(str), begin(buffer));
    auto new_buffer = KeyBuffer{};
    _ultostr(new_buffer, value);
    CHECKS(buffer == new_buffer, "NumToBuffer");
    return buffer;
}

class SystemManagementController {
public:
    SystemManagementController();
    ~SystemManagementController();
    SystemManagementController(SystemManagementController &) = delete;
    SystemManagementController(SystemManagementController &&) = delete;
    SystemManagementController &operator=(const SystemManagementController &) = delete;
    SystemManagementController &operator=(SystemManagementController &&) = delete;

    optional<double> GetCpuTemperature() const noexcept { return cpu_temperature_; }
    optional<double> GetGpuTemperature() const noexcept { return gpu_temperature_; }

    void ReadTemperatures();

private:
    void Open();
    void Close();

    void ReadCpuTemperature();
    void ReadGpuTemperature();

    kern_return_t ConnectCall(int index, const KeyData &input, KeyData &output);
    kern_return_t GetKeyInfo(uint32_t key, KeyInfo &key_info);
    kern_return_t ReadKey(const KeyBuffer &key, Value &value);
    optional<double> GetSP78Value(const Value &value) const;

    io_connect_t service_handle_ = 0;
    optional<double> cpu_temperature_;
    bool cpu_temperature_initialized_ = false;
    optional<double> gpu_temperature_;
    bool gpu_temperature_initialized_ = false;
};

SystemManagementController::SystemManagementController() {
    static_assert(std::is_same<UInt16, uint16_t>::value, "ensure UInt16 and uint16_t are identical");
    static_assert(std::is_same<SInt16, int16_t>::value, "ensure SInt16 and int16_t are identical");
    static_assert(std::is_same<UInt32, uint32_t>::value, "ensure UInt32 and uint32_t are identical");
    Open();
}

SystemManagementController::~SystemManagementController() {
    Close();
}

void SystemManagementController::Open() {
    mach_port_t master_port = 0;

#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) &&                                                          \
    __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= MAC_OS_VERSION_12_0
    IOMainPort(MACH_PORT_NULL, &master_port);
#else
    IOMasterPort(MACH_PORT_NULL, &master_port);
#endif

    CFMutableDictionaryRef matching_dict = IOServiceMatching("AppleSMC");

    io_iterator_t matching_services = 0;
    kern_return_t result = IOServiceGetMatchingServices(master_port, matching_dict, &matching_services);
    if (result != kIOReturnSuccess) {
        THROW_RUNTIME_ERROR("IOServiceGetMatchingServices failed: {:08x}", result);
    }

    io_object_t device = IOIteratorNext(matching_services);
    IOObjectRelease(matching_services);
    if (device == 0) {
        THROW_RUNTIME_ERROR("IOIteratorNext failed to return a device");
    }

    result = IOServiceOpen(device, mach_task_self(), 0, &service_handle_);
    IOObjectRelease(device);
    if (result != kIOReturnSuccess) {
        THROW_RUNTIME_ERROR("IOServiceOpen failed: {:08x}", result);
    }
}

void SystemManagementController::Close() {
    if (service_handle_ != 0) {
        IOServiceClose(service_handle_);
    }
}

kern_return_t SystemManagementController::GetKeyInfo(uint32_t key, KeyInfo &key_info) {
    KeyData input;
    input.key = key;
    input.data8 = READ_KEYINFO_CMD;
    KeyData output;

    kern_return_t result = ConnectCall(KERNEL_INDEX, input, output);
    if (result == kIOReturnSuccess) {
        key_info = output.key_info;
    } else {
        LOG_ERROR("calling IOConnectCallStructMethod with READ_KEYINFO_CMD returned {:08x}", result);
    }
    return result;
}

kern_return_t SystemManagementController::ConnectCall(int index, const KeyData &input, KeyData &output) {
    PRECONDITIONS(service_handle_ != 0, "service handle must be valid");
    size_t input_size = sizeof(KeyData);
    size_t output_size = sizeof(KeyData);
    return IOConnectCallStructMethod(service_handle_, index, &input, input_size, &output, &output_size);
}

kern_return_t SystemManagementController::ReadKey(const KeyBuffer &key, Value &value) {
    KeyData input;
    input.key = BufferToNum(key, 16);
    KeyData output;
    value = Value{};
    value.key = key;

    kern_return_t result = GetKeyInfo(input.key, output.key_info);
    if (result != kIOReturnSuccess) {
        LOG_ERROR("calling GetKeyInfo returned {:08x}", result);
        return result;
    }

    value.data_size = output.key_info.data_size;
    value.data_type = NumToBuffer(output.key_info.data_type);
    input.key_info.data_size = value.data_size;
    input.data8 = READ_BYTES_CMD;

    result = ConnectCall(KERNEL_INDEX, input, output);
    if (result != kIOReturnSuccess) {
        LOG_ERROR("calling IOConnectCallStructMethod with READ_BYTES_CMD returned {:08x}", result);
        return result;
    }

    value.bytes = output.bytes;
    return kIOReturnSuccess;
}

optional<double> SystemManagementController::GetSP78Value(const Value &value) const {
    if (strcmp(data(value.data_type), DATATYPE_SP78) == 0 && value.data_size == 2) {
        auto network_sp78_value = uint16_t{0};
        memcpy(&network_sp78_value, data(value.bytes), sizeof(uint16_t));
        auto host_sp78_value = ntohs(network_sp78_value);
        return static_cast<double>(host_sp78_value) / 256.0;
    } else {
        LOG_ERROR("input value is not SP78, type: {}, length: {}",
                  quoted(string(data(value.data_type))),
                  value.data_size);
        return nullopt;
    }
}

void SystemManagementController::ReadTemperatures() {
    if (!cpu_temperature_initialized_) {
        cpu_temperature_initialized_ = true;
        ReadCpuTemperature();
    }
    if (!gpu_temperature_initialized_) {
        gpu_temperature_initialized_ = true;
        ReadGpuTemperature();
    }
}

void SystemManagementController::ReadCpuTemperature() {
    static constexpr const array<char, 5> CPU_TEMPERATURE_KEY{"TC0P"};
    Value value;
    auto result = ReadKey(CPU_TEMPERATURE_KEY, value);
    if (result != kIOReturnSuccess) {
        LOG_ERROR("calling ReadKey returned {:08x}", result);
        return;
    }
    auto cpu_temperature_holder = GetSP78Value(value);
    if (!cpu_temperature_holder) {
        LOG_ERROR("error while getting SP78 value", result);
        return;
    }
    cpu_temperature_ = cpu_temperature_holder;
    LOG_DEBUG("cpu temperature: {:.1f}°C", *cpu_temperature_);
    return;
}

void SystemManagementController::ReadGpuTemperature() {
    static constexpr const array<char, 5> GPU_TEMPERATURE_KEY{"TG0P"};
    Value value;
    auto result = ReadKey(GPU_TEMPERATURE_KEY, value);
    if (result != kIOReturnSuccess) {
        LOG_ERROR("calling ReadKey returned {:08x}", result);
        return;
    }
    auto gpu_temperature_holder = GetSP78Value(value);
    if (!gpu_temperature_holder) {
        LOG_ERROR("error while getting SP78 value", result);
        return;
    }
    gpu_temperature_ = gpu_temperature_holder;
    LOG_DEBUG("gpu temperature: {:.1f}°C", *gpu_temperature_);
    return;
}

pair<optional<double>, optional<double>> GetCpuGpuTemperatures() {
    static auto smc = SystemManagementController{};
    smc.ReadTemperatures();
    return make_pair(smc.GetCpuTemperature(), smc.GetGpuTemperature());
}

} // namespace

namespace mmotd::platform {

Temperature GetCpuTemperature() {
    auto [cpu_temperature, _] = GetCpuGpuTemperatures();
    if (!cpu_temperature) {
        return Temperature{};
    } else {
        return Temperature{*cpu_temperature, Temperature::Units::Celsius};
    }
}

Temperature GetGpuTemperature() {
    auto [_, gpu_temperature] = GetCpuGpuTemperatures();
    if (!gpu_temperature) {
        return Temperature{};
    } else {
        return Temperature{*gpu_temperature, Temperature::Units::Celsius};
    }
}

} // namespace mmotd::platform
#endif
