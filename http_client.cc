#include "http_client.h"
#include "response.h"
#include "request.h"

#include <string>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

// http://www.boost.org/doc/libs/1_49_0/doc/html/boost_asio/example/http/client/sync_client.cpp
// http://charette.no-ip.com:81/programming/doxygen/boost/group__connect.html#gac44f151131b02a286d4ef5d93d95869b
bool HTTPClient::EstablishConnection(boost::asio::ip::tcp::socket* socket, const std::string& host, const std::string& service) {
	boost::asio::io_service io_service;

	// Get a list of endpoints corresponding to the server name.
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query(host, service);
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query), end;

	// Try each endpoint until we successfully establish a connection.
	boost::system::error_code ec;
	socket_ = boost::asio::ip::tcp::socket(io_service);
	boost::asio::connect(socket_, endpoint_iterator, end, ec);
	if(ec) {
		//error_code means binding the socket failed
		BOOST_LOG_TRIVIAL(error) << "Unable to bind socket to website " + host + " on port " + service;
		return false;
	}
	BOOST_LOG_TRIVIAL(trace) << "===== connection opened for reverse proxy =====";
	return true;
}

std::unique_ptr<Response> HTTPClient::SendRequest(const Request& req) {
	boost::asio::streambuf request;
	std::ostream request_stream(&request);
	request_stream << req.to_string();

	BOOST_LOG_TRIVIAL(trace) << "===== socket written to for reverse proxy =====";
	boost::asio::write(socket_, request);

	// Now, we read data back.
    boost::asio::streambuf raw_response;
	boost::system::error_code ec;

	boost::asio::read(socket_, raw_response, ec);
	if(ec && ec != boost::asio::error::eof) {
		// Error reading.
		return nullptr;
	}

	boost::asio::streambuf::const_buffers_type bufs = raw_response.data();
	auto resp = Response::Parse(std::string(
								boost::asio::buffers_begin(bufs), 
								boost::asio::buffers_begin(bufs) + raw_response.size())
								);
	return resp;

}