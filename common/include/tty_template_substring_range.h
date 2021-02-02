// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"

#include <iterator>
#include <optional>
#include <string>

namespace mmotd::tty_template::tty_string {

class SubstringRange {
    friend bool operator==(const SubstringRange &a, const SubstringRange &b);

public:
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_VIRTUAL_DESTRUCTOR(SubstringRange);

    SubstringRange(size_t position, size_t count);

    SubstringRange(std::string::const_iterator begin,
                   std::string::const_iterator substr_begin,
                   std::string::const_iterator substr_end);

    size_t position() const noexcept { return position_; }
    size_t size() const noexcept { return position_ + count_; }

    bool match(const std::string &txt) const { return IsValidSubstr(txt); }
    bool empty() const noexcept { return position_ == 0ull && (count_ == 0ull || count_ == INVALID_VALUE); }

    void clear() noexcept { position_ = count_ = 0ull; }

    std::optional<std::string> substr(const std::string &txt) const;

private:
    bool IsValidSubstr(const std::string &txt) const;

    size_t position_ = 0ull;
    size_t count_ = INVALID_VALUE;

    inline static constexpr size_t INVALID_VALUE = std::string::npos;
};

} // namespace mmotd::tty_template::tty_string
