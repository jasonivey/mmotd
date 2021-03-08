// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/platform/system_information.h"

#include <iterator>
#include <string>

using namespace std;

namespace mmotd::platform {

bool SystemDetails::empty() const noexcept {
    return std::empty(host_name) && std::empty(computer_name) && std::empty(kernel_version) &&
           std::empty(kernel_release) && std::empty(kernel_type) && std::empty(architecture_type) &&
           std::empty(byte_order) && std::empty(platform_version) && std::empty(platform_name);
}

} // namespace mmotd::platform
