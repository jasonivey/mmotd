// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(_WIN32)
#include "lib/include/platform/load_average.h"

#include <optional>

using namespace std;

namespace mmotd::platform {

LoadAverageDetails GetLoadAverageDetails() {
    return nullopt;
}

} // namespace mmotd::platform
#endif
