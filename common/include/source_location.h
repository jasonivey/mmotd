// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"

#include <cstdint>
#include <string>

namespace mmotd::source_location {

class SourceLocation {
    friend std::string to_string(const SourceLocation &source_location);

public:
    SourceLocation(const char *file, long line, const char *function);
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(SourceLocation);

private:
    std::string to_string() const;

    std::uint32_t line_ = 0;
    // std::uint32_t column_ = 0;
    std::string file_name_;
    std::string function_name_;
};

} // namespace mmotd::source_location
