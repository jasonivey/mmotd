// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(_WIN32)
#include "common/include/human_size.h"
#include "common/include/posix_error.h"
#include "lib/include/platform/memory.h"

using namespace std;

namespace mmotd::platform {

MemoryDetails GetMemoryDetails() {
    return MemoryDetails{};
}

} // namespace mmotd::platform
#endif
