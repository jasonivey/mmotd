// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/http_request.h"

#include <boost/exception/all.hpp>
#include <boost/system/error_code.hpp>
#include <boost/beast/http/error.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/log/trivial.hpp>
#include <fmt/format.h>
#include <cstdlib>
#include <string>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>
using namespace std;

HttpRequest::HttpRequest(HttpProtocol protocol, string host, string port) :
    protocol_(protocol),
	host_(host),
	port_(port.empty() ? (protocol_ == HttpProtocol::HTTPS ? "443" : "80") : port) {}

string HttpRequest::MakeRequest(string path) {
	try {
        auto response = optional<string>{};
        if (protocol_ == HttpProtocol::HTTPS) {
            response = TryMakeSecureRequest(path);
        } else {
            response = TryMakeRequest(path);
        }
        return response == nullopt ? string{} : *response;
	} catch (const exception &ex) {
        BOOST_LOG_TRIVIAL(error) << "exception " << ex.what();
        return string{};
    }
}

optional<string> HttpRequest::TryMakeRequest(std::string path) {
    const auto version = int{11};

    // The io_context is required for all I/O
    net::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    // Look up the domain name
    BOOST_LOG_TRIVIAL(info) << fmt::format("resolving {}:{} for an actual address", host_, 80);
    auto const results = resolver.resolve(host_, port_);

    // Make the connection on the IP address we get from a lookup
    BOOST_LOG_TRIVIAL(info) << fmt::format("connecting to {}:{}", host_, port_);
    stream.connect(results);

    // Set up an HTTP GET request message
    http::request<http::string_body> request{http::verb::get, "/json", version};
    request.set(http::field::host, host_);
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    request.set(http::field::content_type, "application/json");

    // Send the HTTP request to the remote host
    http::write(stream, request);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::dynamic_body> response;

    // Receive the HTTP response
    BOOST_LOG_TRIVIAL(info) << fmt::format("reading the response from to {}:{}{}", host_, port_, path);
    try {
        http::read(stream, buffer, response);
    } catch (boost::system::error_code &ec) {
        if (ec == http::error::end_of_stream) {
            BOOST_LOG_TRIVIAL(info) << fmt::format("ignoring 'http::error::end_of_stream' during http read of {}:{}{}, {}", host_, port_, path, ec.message());
            ec = boost::system::error_code{};
        } else {
            BOOST_LOG_TRIVIAL(error) << fmt::format("during http read of {}:{}{}, {}", host_, port_, path, ec.message());
            return nullopt;
        }
    } catch (boost::exception &ex) {
        BOOST_LOG_TRIVIAL(error) << fmt::format("during http read of {}:{}{}, {}", host_, port_, path, boost::diagnostic_information(ex));
    } catch (std::exception &ex) {
        BOOST_LOG_TRIVIAL(error) << fmt::format("during http read of {}:{}{}, {}", host_, port_, path, ex.what());
    }

    if (response.result() != http::status::ok) {
        BOOST_LOG_TRIVIAL(error) << fmt::format("http status: {}, {}", response.result_int(), response.reason().to_string());
        return nullopt;
    }

    // Write the message to standard out
    auto response_str = beast::buffers_to_string(response.body().data());
    BOOST_LOG_TRIVIAL(info) << fmt::format("response from {}:{}{}\n{}", host_, port_, path, response_str);

    // Gracefully close the socket
    BOOST_LOG_TRIVIAL(info) << fmt::format("shutting down the socket to {}:{}", host_, port_);
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes
    // so don't bother reporting it.
    //
    if (ec && ec != beast::errc::not_connected)
        throw beast::system_error{ec};

    // If we get here then the connection is closed gracefully
    return make_optional(response_str);
}

#if 0
optional<string> HttpRequest::TryMakeSecureRequest(string path) {
        // The io_context is required for all I/O
        net::io_context ioc;

        // The SSL context is required, and holds certificates
        ssl::context ctx(ssl::context::tlsv12_client);

        // This holds the root certificate used for verification
        load_root_certificates(ctx);

        // Verify the remote server's certificate
        ctx.set_verify_mode(ssl::verify_peer);

            // These objects perform our I/O
        tcp::resolver resolver(ioc);
        beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if(! SSL_set_tlsext_host_name(stream.native_handle(), host))
        {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
            throw beast::system_error{ec};
        }

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(stream).connect(results);

        // Perform the SSL handshake
        stream.handshake(ssl::stream_base::client);

        // Set up an HTTP GET request message
        http::request<http::string_body> req{http::verb::get, target, version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Send the HTTP request to the remote host
        http::write(stream, req);

        // This buffer is used for reading and must be persisted
        beast::flat_buffer buffer;

        // Declare a container to hold the response
        http::response<http::dynamic_body> res;

        // Receive the HTTP response
        http::read(stream, buffer, res);

        // Write the message to standard out
        std::cout << res << std::endl;

        // Gracefully close the stream
        beast::error_code ec;
        stream.shutdown(ec);
        if(ec == net::error::eof)
        {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec = {};
        }
        if(ec)
            throw beast::system_error{ec};

        // If we get here then the connection is closed gracefully
}
#endif
