// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include "lib/include/platform/network.h"

#include <string>
#include <vector>

namespace mmotd::platform {

NetworkDetails GetNetworkDetails() {
    return NetworkDetails{};
}

} // namespace mmotd::platform

#endif
