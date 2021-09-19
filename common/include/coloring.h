// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <string>

namespace mmotd::core::colors {

std::string RemoveNonAsciiCharsCopy(std::string input);
void RemoveNonAsciiChars(std::string &str);

} // namespace mmotd::core::emoji
