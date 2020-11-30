#include "common/include/chrono-io.h"

#include <fmt/format.h>

using namespace std;
using fmt::format;

namespace mmotd { namespace chrono { namespace io {

string to_string(std::chrono::system_clock /*system_clock*/, string_view /*chrono_format*/) {
    return string{};
    //return format(chrono_format, system_clock);
}

}}} // namespace mmotd::chrono::io

