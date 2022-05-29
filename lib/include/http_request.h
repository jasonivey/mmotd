// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <optional>
#include <string>
#include <string_view>

namespace mmotd::networking {

enum class HttpProtocol { HTTP, HTTPS };

class HttpRequest {
public:
    static std::optional<std::string> Get(HttpProtocol protocol,
                                          std::string_view host,
                                          std::string_view path = std::string_view{},
                                          std::string_view query = std::string_view{});

    static std::string GetUrl(HttpProtocol protocol,
                              std::string_view host,
                              std::string_view path = std::string_view{},
                              std::string_view query = std::string_view{},
                              bool url_encode = false);
};

} // namespace mmotd::networking
