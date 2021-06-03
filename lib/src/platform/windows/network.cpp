// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(_WIN32)
#include "lib/include/platform/network.h"

#include <string>
#include <vector>

namespace mmotd::platform {

NetworkDetails GetNetworkDetails() {
    return NetworkDetails{};
}

} // namespace mmotd::platform
#endif
