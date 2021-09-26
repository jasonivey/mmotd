// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <exception>
#include <string>

#include <catch2/catch.hpp>

namespace mmotd::test {

struct MmotdExceptionMatcher : Catch::MatcherBase<std::exception> {
    std::string text_;

    MmotdExceptionMatcher(char const *text);

    ~MmotdExceptionMatcher() override;

    bool match(std::exception const &arg) const override;

    std::string describe() const override;
};

} // namespace mmotd::test
