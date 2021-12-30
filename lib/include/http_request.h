// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"

#include <cstdint>
#include <optional>
#include <string>

namespace mmotd::networking {

enum class HttpProtocol { HTTP, HTTPS };

std::string to_string(HttpProtocol protocol);

class HttpRequest {
public:
    DEFAULT_DESTRUCTOR_DELETE_CONSTRUCTORS_COPY_MOVE_OPERATORS(HttpRequest);

    HttpRequest(HttpProtocol protocol, std::string host, std::string port = std::string{});

    std::optional<std::string> MakeRequest(std::string path, std::string path_w_o_auth);

private:
    HttpProtocol protocol_ = HttpProtocol::HTTPS;
    std::string host_;
    std::string port_ = "443";
};

} // namespace mmotd::networking
