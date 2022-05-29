// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/http_request.h"

#include "common/assertion/include/assertion.h"
#include "common/include/algorithm.h"
#include "common/include/logging.h"

#include <algorithm>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <curl/curl.h>
#include <curl/urlapi.h>
#include <fmt/format.h>
#include <scope_guard.hpp>
#include <spdlog/fmt/bin_to_hex.h>

using mmotd::networking::HttpProtocol;
using namespace std;

namespace {

/* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
static std::mutex g_request_mutex;

string GetUserAgent() {
    const char *cversion = curl_version();
    LOG_VERBOSE("CURL version: {}", cversion);
    auto curl_abrv_version = string{string_view{cversion}.substr(0, string_view{cversion}.find(' '))};
    LOG_VERBOSE("CURL abbreviated version: '{}'", curl_abrv_version);
    return curl_abrv_version;
}

#if defined(MMOTD_HTTP_VERBOSE_LOGGING)
int CurlDebugFunction(CURL *, curl_infotype type, char *data, size_t size, void *) {
    auto data_buf = vector<uint8_t>(size);
    memcpy(std::data(data_buf), data, size);
    switch (type) {
        case CURLINFO_TEXT:
            LOG_VERBOSE("CURL: {}", string(data, data + size));
            break;
        case CURLINFO_HEADER_OUT:
            LOG_VERBOSE("CURL sending header ({} bytes): {:a}", size, spdlog::to_hex(data_buf, size_t{16}));
            break;
        case CURLINFO_DATA_OUT:
            LOG_VERBOSE("CURL sending data ({} bytes): {:a}", size, spdlog::to_hex(data_buf, size_t{16}));
            break;
        case CURLINFO_SSL_DATA_OUT:
            LOG_VERBOSE("CURL sending SSL data ({} bytes): {:a}", size, spdlog::to_hex(data_buf, size_t{16}));
            break;
        case CURLINFO_HEADER_IN:
            LOG_VERBOSE("CURL receiving header ({} bytes): {:a}", size, spdlog::to_hex(data_buf, size_t{16}));
            break;
        case CURLINFO_DATA_IN:
            LOG_VERBOSE("CURL receiving data ({} bytes): {:a}", size, spdlog::to_hex(data_buf, size_t{16}));
            break;
        case CURLINFO_SSL_DATA_IN:
            LOG_VERBOSE("CURL receiving SSL data ({} bytes): {:a}", size, spdlog::to_hex(data_buf, size_t{16}));
            break;
        case CURLINFO_END:
        default:
            break;
    }
    return 0;
}
#else
inline int CurlDebugFunction(CURL *, curl_infotype, char *, size_t, void *) {
    return 0;
}
#endif

template<typename T>
bool CurlSetOption(CURL *curl, CURLoption opt, T val) {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    if (auto retval = curl_easy_setopt(curl, opt, val); retval != CURLE_OK) {
        LOG_ERROR("failed to set curl option '{}', error: {}: {}", opt, retval, curl_easy_strerror(retval));
        return false;
    }
    return true;
}

bool CurlSetOption(CURL *curl, CURLoption opt, string val) {
    LOG_VERBOSE("setting curl option value: {}", val);
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    if (auto retval = curl_easy_setopt(curl, opt, data(val)); retval != CURLE_OK) {
        LOG_ERROR("failed to set curl option '{}', error: {}: {}", opt, retval, curl_easy_strerror(retval));
        return false;
    }
    return true;
}

size_t CurlWriteFunction(void *ptr, size_t size, size_t nmemb, string *data) {
    if (ptr == nullptr || data == nullptr || size == 0 || nmemb == 0) {
        LOG_ERROR("invalid input, ptr={}, data={}, size={}, nmemb={}", fmt::ptr(ptr), fmt::ptr(data), size, nmemb);
        return 0;
    }
    data->append(static_cast<char *>(ptr), size * nmemb);
    return size * nmemb;
}

} // namespace

namespace mmotd::networking {

class HttpClient {
public:
    HttpClient();
    HttpClient(string url);
    ~HttpClient();
    HttpClient(HttpClient &other) = delete;
    HttpClient(HttpClient &&other) = delete;
    HttpClient &operator=(HttpClient &other) = delete;
    HttpClient &operator=(HttpClient &&other) = delete;

    optional<string> Get();

private:
    CURL *curl_ = nullptr;
    string url_;
};

HttpClient::HttpClient() = default;

HttpClient::HttpClient(string url) : curl_(curl_easy_init()), url_(move(url)) {
    if (curl_ == nullptr) {
        THROW_RUNTIME_ERROR("curl_easy_init() returned {}", "nullptr");
        return;
    }
}

HttpClient::~HttpClient() {
    if (curl_ != nullptr) {
        curl_easy_cleanup(curl_);
    }
}

optional<string> HttpClient::Get() {
    PRECONDITIONS(curl_ != nullptr, "curl handle must be valid");
    PRECONDITIONS(!empty(url_), "url must be valid");

    LOG_VERBOSE("url: {}", url_);

    auto response = string{};
    if (!CurlSetOption(curl_, CURLOPT_HTTPGET, 1L) ||               // ask for an HTTP GET request
        !CurlSetOption(curl_, CURLOPT_URL, data(url_)) ||           // URL in CURLU * format
        !CurlSetOption(curl_, CURLOPT_FAILONERROR, 1L) ||           // request failure on HTTP response >= 400
        !CurlSetOption(curl_, CURLOPT_USERAGENT, GetUserAgent()) || // user agent
        !CurlSetOption(curl_, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_NONE) || // HTTP protocol version to use
        !CurlSetOption(curl_, CURLOPT_TCP_KEEPALIVE, 1L) ||                    // TCP keep-alive probing
        !CurlSetOption(curl_, CURLOPT_FOLLOWLOCATION, 1L) ||                   // follow HTTP 3xx redirects
        !CurlSetOption(curl_, CURLOPT_MAXREDIRS, 50L) ||                       // maximum number of redirects allowed
        !CurlSetOption(curl_, CURLOPT_NOPROGRESS, 1L) ||                       // switch off the progress meter
        !CurlSetOption(curl_, CURLOPT_VERBOSE, 1L) ||                          // verbose mode
        !CurlSetOption(curl_, CURLOPT_DEBUGFUNCTION, CurlDebugFunction) ||     // debug callback
        !CurlSetOption(curl_, CURLOPT_WRITEFUNCTION, CurlWriteFunction) ||     // callback for writing received data
        !CurlSetOption(curl_, CURLOPT_WRITEDATA, &response)) {                 // pointer passed to the write callback
        return nullopt;
    }

    auto ret_code = curl_easy_perform(curl_);
    if (ret_code != CURLE_OK) {
        LOG_ERROR("failed when calling curl_easy_perform, error {}: {}", ret_code, curl_easy_strerror(ret_code));
        return nullopt;
    }

    return std::empty(response) ? nullopt : make_optional(response);
}

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

optional<string> HttpRequest::Get(HttpProtocol protocol, string_view host, string_view path, string_view query) {
    auto url = HttpRequest::GetUrl(protocol, host, path, query, true);
    scoped_lock<mutex> lock(g_request_mutex);
    return HttpClient{url}.Get();
}

string
HttpRequest::GetUrl(HttpProtocol protocol, string_view host, string_view path, string_view query, bool url_encode) {
    auto url = string{};
    if (!empty(path) && !empty(query)) {
        url = fmt::format(FMT_STRING("{}://{}/{}?{}"), to_string(protocol), host, path, query);
    } else if (!empty(path)) {
        url = fmt::format(FMT_STRING("{}://{}/{}"), to_string(protocol), host, path);
    } else if (!empty(query)) {
        url = fmt::format(FMT_STRING("{}://{}?{}"), to_string(protocol), host, query);
    } else {
        url = fmt::format(FMT_STRING("{}://{}"), to_string(protocol), host);
    }
    if (url_encode) {
        boost::replace_all(url, "%", "%25");
        boost::replace_all(url, " ", "%20");
    }
    return url;
}

} // namespace mmotd::networking
