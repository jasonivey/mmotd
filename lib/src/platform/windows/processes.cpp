// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include "lib/include/platform/processes.h"

using namespace std;

namespace mmotd::platform {

optional<size_t> GetProcessCount() {
    return nullopt;
}

} // namespace mmotd::platform

#endif