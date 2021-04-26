// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/logging.h"
#include "lib/include/http_request.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <fmt/format.h>

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace ssl = asio::ssl;
namespace http = boost::beast::http;
using mmotd::networking::HttpProtocol;
using tcp = boost::asio::ip::tcp;
using ssl_stream = ssl::stream<tcp::socket>;
using ssl_stream_ptr = std::unique_ptr<ssl_stream>;
using tcp_stream_ptr = std::unique_ptr<beast::tcp_stream>;
using fmt::format;
using namespace std;

namespace {

class HttpClient {
public:
    HttpClient() = default;
    explicit HttpClient(HttpProtocol protocol, const string &host, const string &port, const string &path);

    optional<string> MakeRequest();

private:
    template<typename T>
    optional<string> MakeRequestImpl();

    template<typename T>
    T CreateStream(asio::io_context &ctx);

    template<typename T>
    void CloseStream(T &stream);

    bool IsHttps() const { return protocol_ == HttpProtocol::HTTPS; }

    tcp::resolver::results_type Resolve(asio::io_context &ctx);
    tcp::socket Connect(asio::io_context &ctx);
    ssl_stream_ptr MakeStream(asio::io_context &ctx, ssl::context &ssl_ctx);
    tcp_stream_ptr MakeStream(asio::io_context &ctx);

    template<typename T>
    http::response<http::string_body> Request(T &stream);
    //http::response<http::string_body> Request(beast::tcp_stream &stream);

    HttpProtocol protocol_ = HttpProtocol::HTTP;
    string host_;
    string port_;
    string path_;
};

HttpClient::HttpClient(HttpProtocol protocol, const string &host, const string &port, const string &path) :
    protocol_(protocol), host_(host), port_(port), path_(path) {
}

tcp::resolver::results_type HttpClient::Resolve(asio::io_context &ctx) {
    auto resolver = tcp::resolver{ctx};
    return resolver.resolve(host_, port_);
}

tcp::socket HttpClient::Connect(asio::io_context &ctx) {
    auto socket = tcp::socket{ctx};
    asio::connect(socket, Resolve(ctx));
    return socket;
}

ssl_stream_ptr HttpClient::MakeStream(asio::io_context &ctx, ssl::context &ssl_ctx) {
    auto stream = boost::make_unique<ssl_stream>(Connect(ctx), ssl_ctx);
    boost::certify::set_server_hostname(*stream, host_);
    boost::certify::sni_hostname(*stream, host_);
    stream->handshake(ssl::stream_base::handshake_type::client);
    return stream;
}

tcp_stream_ptr HttpClient::MakeStream(asio::io_context &ctx) {
    return boost::make_unique<beast::tcp_stream>(Connect(ctx));
}

#if 0
http::response<http::string_body> HttpClient::Request(ssl_stream &stream) {
    http::request<http::empty_body> request;
    request.method(http::verb::get);
    request.target(path_);
    request.keep_alive(false);
    request.version(11);
    request.set(http::field::host, host_);
    http::write(stream, request);

    http::response<http::string_body> response;
    beast::flat_buffer buffer;
    http::read(stream, buffer, response);

    return response;
}
#endif

template<typename T>
http::response<http::string_body> HttpClient::Request(T &stream) {
    http::request<http::empty_body> request;
    request.method(http::verb::get);
    request.target(path_);
    request.keep_alive(false);
    request.version(11);
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    request.set(http::field::content_type, "application/json");
    request.set(http::field::host, host_);
    http::write(stream, request);

    http::response<http::string_body> response;
    beast::flat_buffer buffer;
    http::read(stream, buffer, response);

    return response;
}

template<>
ssl_stream_ptr HttpClient::CreateStream<ssl_stream_ptr>(asio::io_context &ctx) {
    ssl::context ssl_ctx{ssl::context::tls_client};
    ssl_ctx.set_verify_mode(ssl::context::verify_peer | ssl::context::verify_fail_if_no_peer_cert);
    ssl_ctx.set_default_verify_paths();

    boost::certify::enable_native_https_server_verification(ssl_ctx);

    return MakeStream(ctx, ssl_ctx);
}

template<>
tcp_stream_ptr HttpClient::CreateStream<tcp_stream_ptr>(asio::io_context &ctx) {
    return MakeStream(ctx);
}

template<>
void HttpClient::CloseStream<ssl_stream>(ssl_stream &stream) {
    auto ec = boost::system::error_code{};
    stream.shutdown(ec);
    stream.next_layer().close(ec);
}

template<>
void HttpClient::CloseStream<beast::tcp_stream>(beast::tcp_stream &stream) {
    auto ec = boost::system::error_code{};
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);
}

template<typename T>
optional<string> HttpClient::MakeRequestImpl() {
    asio::io_context ctx;

    auto stream = CreateStream<T>(ctx);
    auto response = Request(*stream);

    auto response_str = response.body();
    LOG_INFO("response from {}:{}{}\n{}", host_, port_, path_, response_str);
    CloseStream(*stream);

    return make_optional(response_str);
}

optional<string> HttpClient::MakeRequest() {
    if (IsHttps()) {
        return MakeRequestImpl<ssl_stream_ptr>();
    } else {
        return MakeRequestImpl<tcp_stream_ptr>();
    }
}

} // namespace

namespace mmotd::networking {

HttpRequest::HttpRequest(HttpProtocol protocol, string host, string port) :
    protocol_(protocol), host_(host), port_(port.empty() ? (protocol_ == HttpProtocol::HTTPS ? "443" : "80") : port) {
}

optional<string> HttpRequest::MakeRequest(string path) {
    return HttpClient{protocol_, host_, port_, path}.MakeRequest();
}

} // namespace mmotd::networking
