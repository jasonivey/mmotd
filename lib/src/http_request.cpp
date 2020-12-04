// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/http_request.h"

#include <cstdlib>
#include <string>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/error.hpp>
#include <boost/beast/version.hpp>
#include <boost/exception/all.hpp>
#include <boost/system/error_code.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using fmt::format;
using namespace std;

HttpRequest::HttpRequest(HttpProtocol protocol, string host, string port) :
    protocol_(protocol), host_(host), port_(port.empty() ? (protocol_ == HttpProtocol::HTTPS ? "443" : "80") : port) {
}

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
        PLOG_ERROR << "exception " << ex.what();
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
    PLOG_DEBUG << format("resolving {}:{} for an actual address", host_, 80);
    auto const results = resolver.resolve(host_, port_);

    // Make the connection on the IP address we get from a lookup
    PLOG_DEBUG << format("connecting to {}:{}", host_, port_);
    stream.connect(results);

    // Set up an HTTP GET request message
    http::request<http::string_body> request{http::verb::get, path, version};
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
    PLOG_DEBUG << format("requesting {}:{}{}", host_, port_, path);
    try {
        http::read(stream, buffer, response);
    } catch (boost::system::error_code &ec) {
        if (ec == http::error::end_of_stream) {
            PLOG_INFO << format("ignoring 'http::error::end_of_stream' during http request of {}:{}{} (details: {})",
                                host_,
                                port_,
                                path,
                                ec.message());
            ec = boost::system::error_code{};
        } else {
            PLOG_ERROR << format("during http request of {}:{}{}, {}", host_, port_, path, ec.message());
            return nullopt;
        }
    } catch (boost::exception &ex) {
        PLOG_ERROR << format("during http request of {}:{}{} (details: {})",
                             host_,
                             port_,
                             path,
                             boost::diagnostic_information(ex));
        return nullopt;
    } catch (std::exception &ex) {
        PLOG_ERROR << format("during http request of {}:{}{} (details: {})", host_, port_, path, ex.what());
        return nullopt;
    }

    if (response.result() != http::status::ok) {
        PLOG_ERROR << format("http request status != 200, status: {}, reason: {}",
                             response.result_int(),
                             response.reason().to_string());
        return nullopt;
    }

    auto response_str = beast::buffers_to_string(response.body().data());
    PLOG_DEBUG << format("response from {}:{}{}\n{}", host_, port_, path, response_str);

    PLOG_DEBUG << format("shutting down the socket to {}:{}", host_, port_);
    auto ec = boost::system::error_code{};
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    if (ec && ec != beast::errc::not_connected) {
        PLOG_ERROR << format("shutting down the socket to {}:{}{} (details: {})", host_, port_, path, ec.message());
        throw beast::system_error{ec};
    }

    return make_optional(response_str);
}
