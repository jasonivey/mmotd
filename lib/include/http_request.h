// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <cstdint>
#include <optional>
#include <string>

enum class HttpProtocol { HTTP, HTTPS };

class HttpRequest {
public:
    HttpRequest(HttpProtocol protocol, std::string host, std::string port = std::string{});

    std::string MakeRequest(std::string path);

private:
    //std::optional<std::string> TryMakeRequest(std::string path);
    std::optional<std::string> TryMakeSecureRequest(HttpProtocol protocol,
                                                    const std::string &host,
                                                    const std::string &port,
                                                    const std::string &path);

    HttpProtocol protocol_;
    std::string host_;
    std::string port_;
};
