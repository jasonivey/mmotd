// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <ios>
#include <string>

#include <fmt/format.h>

namespace mmotd::error::ios_flags {

inline std::string to_string(const std::ios &strm) {
    return fmt::format("good()={}, eof()={}, fail()={}, bad()={}", strm.good(), strm.eof(), strm.fail(), strm.bad());
}

} // namespace mmotd::error::ios_flags
