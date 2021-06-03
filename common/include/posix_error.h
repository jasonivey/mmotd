// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <cerrno>
#include <string>
#include <system_error>

namespace mmotd::error::posix_error {

inline std::string to_string(int err_code) {
    return err_code == 0 ? std::string{} : std::make_error_code(static_cast<std::errc>(err_code)).message();
}

inline std::string to_string() {
    return errno == 0 ? std::string{} : std::make_error_code(static_cast<std::errc>(errno)).message();
}

} // namespace mmotd::error::posix_error
