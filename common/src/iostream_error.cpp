// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/iostream_error.h"

#include <ios>
#include <string>

#include <fmt/format.h>
#include <plog/Log.h>

using namespace std;
using fmt::format;

namespace mmotd::error::ios_flags {

string to_string(const std::ios &strm) {
    return format("good()={}, eof()={}, fail()={}, bad()={}", strm.good(), strm.eof(), strm.fail(), strm.bad());
}

} // namespace mmotd::error::ios_flags
