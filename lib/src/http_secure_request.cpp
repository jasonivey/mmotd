// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/http_request.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/write.hpp>

#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>

#include <boost/log/trivial.hpp>
#include <fmt/format.h>

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace ssl = asio::ssl;
namespace http = boost::beast::http;
using tcp = boost::asio::ip::tcp;
using ssl_stream = ssl::stream<tcp::socket>;
using stream_ptr = std::unique_ptr<ssl_stream>;
using namespace std;

namespace {
tcp::resolver::results_type resolve(asio::io_context& ctx, std::string const& hostname) {
    auto resolver = tcp::resolver{ctx};
    return resolver.resolve(hostname, "https");
}

tcp::socket connect(asio::io_context& ctx, std::string const& hostname) {
    auto socket = tcp::socket{ctx};
    asio::connect(socket, resolve(ctx, hostname));
    return socket;
}

 stream_ptr connect(asio::io_context& ctx, ssl::context& ssl_ctx, std::string const& hostname) {
    auto stream = boost::make_unique<ssl_stream>(connect(ctx, hostname), ssl_ctx);
    // tag::stream_setup_source[]
    boost::certify::set_server_hostname(*stream, hostname);
    boost::certify::sni_hostname(*stream, hostname);
    // end::stream_setup_source[]

    stream->handshake(ssl::stream_base::handshake_type::client);
    return stream;
}

http::response<http::string_body> get(ssl_stream& stream, boost::string_view hostname, boost::string_view uri) {
    http::request<http::empty_body> request;
    request.method(http::verb::get);
    request.target(uri);
    request.keep_alive(false);
    request.set(http::field::host, hostname);
    http::write(stream, request);

    http::response<http::string_body> response;
    beast::flat_buffer buffer;
    http::read(stream, buffer, response);

    return response;
}
}

optional<string> HttpRequest::TryMakeSecureRequest(string path) {
    asio::io_context ctx;
    ssl::context ssl_ctx{ssl::context::tls_client};
    ssl_ctx.set_verify_mode(ssl::context::verify_peer | ssl::context::verify_fail_if_no_peer_cert);
    ssl_ctx.set_default_verify_paths();
    // tag::ctx_setup_source[]
    boost::certify::enable_native_https_server_verification(ssl_ctx);
    // end::ctx_setup_source[]
    auto stream = connect(ctx, ssl_ctx, host_);
    auto response = get(*stream, host_, path);

    auto response_str = response.body();
    BOOST_LOG_TRIVIAL(info) << fmt::format("response from {}:{}{}\n{}", host_, port_, path, response_str);

    boost::system::error_code ec;
    stream->shutdown(ec);
    stream->next_layer().close(ec);

    return make_optional(response_str);
}
