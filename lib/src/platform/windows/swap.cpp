// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include "lib/include/platform/swap.h"

using namespace std;

namespace mmotd::platform {

SwapDetails GetSwapDetails() {
    return SwapDetails{};
}

} // namespace mmotd::platform

#endif
