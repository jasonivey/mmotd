// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <cstdint>
#include <string>
#include <tuple>

class HttpRequest {
public:
    HttpRequest(std::string host, std::string port = std::string{});

    std::string MakeRequest(std::string path);

private:
    std::tuple<bool, std::string> TryMakeRequest(std::string path);

    std::string Host_;
    std::string Port_;
};
