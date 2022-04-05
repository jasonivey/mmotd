// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/http_request.h"

#include "common/include/logging.h"

#include <utility>

#include <curl/curl.h>
#include <fmt/format.h>
#include <scope_guard.hpp>

using fmt::format;
using mmotd::networking::HttpProtocol;
using namespace std;

namespace {

class HttpClient {
public:
    HttpClient() = default;
    HttpClient(HttpProtocol protocol, string host, string port, string path, string path_no_auth);

    optional<string> MakeRequest();

private:
    bool IsHttps() const { return protocol_ == HttpProtocol::HTTPS; }
    string to_string() const;

    HttpProtocol protocol_ = HttpProtocol::HTTP;
    string host_;
    string port_;
    string path_;
    string path_no_auth_;
};

HttpClient::HttpClient(HttpProtocol protocol, string host, string port, string path, string path_no_auth) :
    protocol_(protocol),
    host_(std::move(host)),
    port_(std::move(port)),
    path_(std::move(path)),
    path_no_auth_(std::move(path_no_auth)) {}

string HttpClient::to_string() const {
    auto protocol_str = mmotd::networking::to_string(protocol_);
    auto port = empty(port_) ? IsHttps() ? "443" : "80" : port_;
    auto path = path_no_auth_;
    if (empty(path)) {
        path = "/";
    }
    if (path.front() != '/') {
        path = "/" + path;
    }
    return format(FMT_STRING("{}://{}:{}{}"), protocol_str, host_, port, path);
}

static size_t CurlWriteFunction(void *ptr, size_t size, size_t nmemb, std::string *data) {
    if (ptr == nullptr || data == nullptr || size == 0 || nmemb == 0) {
        LOG_ERROR("invalid input, ptr={}, data={}, size={}, nmemb={}", fmt::ptr(ptr), fmt::ptr(data), size, nmemb);
        return 0;
    }
    data->append(static_cast<char *>(ptr), size * nmemb);
    return size * nmemb;
}

#define CURL_EASY_SETOPT(hndl, opt, val)                                                                               \
    do {                                                                                                               \
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */                                                        \
        if (auto ec = curl_easy_setopt(hndl, opt, val); ec != CURLE_OK) {                                              \
            LOG_ERROR("failed to set curl option '{}', error: {}: {}", #opt, ec, curl_easy_strerror(ec));              \
            return nullopt;                                                                                            \
        }                                                                                                              \
    } while (0)

optional<string> HttpClient::MakeRequest() {
    // fix: todo: jasoni: the global init and cleanup should be done once per process (i.e. main thread)
    auto ret_code = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (ret_code != CURLE_OK) {
        LOG_ERROR("failed when calling 'curl_global_init(CURL_GLOBAL_DEFAULT)', error: {}: {}",
                  ret_code,
                  curl_easy_strerror(ret_code));
        return nullopt;
    }
    sg::make_scope_guard([]() noexcept { curl_global_cleanup(); });

    auto *curl_handle = curl_easy_init();
    if (curl_handle == nullptr) {
        LOG_ERROR("curl_easy_init did not return a valid handle (hndl == nullptr)");
        return nullopt;
    }
    sg::make_scope_guard([&curl_handle]() noexcept { curl_easy_cleanup(curl_handle); });

    const auto url = to_string();
    CURL_EASY_SETOPT(curl_handle, CURLOPT_URL, std::data(url));
    CURL_EASY_SETOPT(curl_handle, CURLOPT_NOPROGRESS, 1L);
    CURL_EASY_SETOPT(curl_handle, CURLOPT_MAXREDIRS, 50L);
    CURL_EASY_SETOPT(curl_handle, CURLOPT_TCP_KEEPALIVE, 1L);
    CURL_EASY_SETOPT(curl_handle, CURLOPT_WRITEFUNCTION, CurlWriteFunction);
    auto response = string{};
    CURL_EASY_SETOPT(curl_handle, CURLOPT_WRITEDATA, &response);

    ret_code = curl_easy_perform(curl_handle);
    if (ret_code != CURLE_OK) {
        LOG_ERROR("failed when calling curl_easy_perform to '{}', error: {}: {}",
                  std::data(url),
                  ret_code,
                  curl_easy_strerror(ret_code));
        return nullopt;
    }

    return std::empty(response) ? nullopt : make_optional(response);
}

} // namespace

namespace mmotd::networking {

string to_string(HttpProtocol protocol) {
    switch (protocol) {
        case HttpProtocol::HTTP:
            return "http";
        case HttpProtocol::HTTPS:
            return "https";
        default:
            return "unknown";
    }
}

HttpRequest::HttpRequest(HttpProtocol protocol, string host, string port) :
    protocol_(protocol),
    host_(std::move(host)),
    port_(port.empty() ? (protocol_ == HttpProtocol::HTTPS ? "443" : "80") : port) {}

optional<string> HttpRequest::MakeRequest(string path, string path_no_auth) {
    return HttpClient{protocol_, host_, port_, path, path_no_auth}.MakeRequest();
}

} // namespace mmotd::networking
