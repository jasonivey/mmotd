// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/test/include/exception_matcher.h"

#include <exception>
#include <iomanip>
#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

using namespace std;

namespace mmotd::test {

MmotdExceptionMatcher::MmotdExceptionMatcher(char const *text) : text_(text) {}

MmotdExceptionMatcher::~MmotdExceptionMatcher() = default;

bool MmotdExceptionMatcher::match(exception const &arg) const {
    return string{arg.what()}.find(text_) != string::npos;
}

string MmotdExceptionMatcher::describe() const {
    return "MmotdExceptionMatcher";
}

} // namespace mmotd::test
