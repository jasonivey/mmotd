// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <string>
#include <tuple>
#include <vector>

#include <fmt/color.h>
#include <fmt/format.h>

namespace mmotd::tty::color {

std::vector<std::tuple<fmt::text_style, std::string>> ParseFormatString(std::string s);

template<typename... Args>
//std::string format(std::string f, const Args &...args) {
std::string format(std::string, const Args &...) {
    return std::string{};
}

template<typename... Args>
//std::string print(std::string f, const Args &...args) {
std::string print(std::string, const Args &...) {
    return std::string{};
}

} // namespace mmotd::tty::color
