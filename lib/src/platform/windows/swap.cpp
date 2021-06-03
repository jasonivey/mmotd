// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(_WIN32)
#include "lib/include/platform/swap.h"

using namespace std;

namespace mmotd::platform {

SwapDetails GetSwapDetails() {
    return SwapDetails{};
}

} // namespace mmotd::platform
#endif
