#include "http_client.h"
#include "response.h"
#include "request.h"

#include <string>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

// http://www.boost.org/doc/libs/1_49_0/doc/html/boost_asio/example/http/client/sync_client.cpp
// http://charette.no-ip.com:81/programming/doxygen/boost/group__connect.html#gac44f151131b02a286d4ef5d93d95869b
bool HTTPClient::EstablishConnection(const std::string& host, const std::string& service) {
	printf("host: %s, service: %s", host.c_str(), service.c_str());
	boost::asio::io_service io_service;

	// Get a list of endpoints corresponding to the server name.
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query(host, service);
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query), end;

	// Try each endpoint until we successfully establish a connection.
	boost::system::error_code ec;
	socket_ = new boost::asio::ip::tcp::socket(io_service);
	
	

	boost::asio::connect(*socket_, endpoint_iterator, end, ec);
	// Nagle's algorithm makes TCP very slow. 
	socket_->set_option(boost::asio::ip::tcp::no_delay(true));

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
	request_stream << req.ToString();

	BOOST_LOG_TRIVIAL(trace) << "===== socket written to for reverse proxy =====";
	boost::asio::write(*socket_, request);


	BOOST_LOG_TRIVIAL(trace) << "===== socket reading for reverse proxy =====";
	// Now, we read data back.
    boost::asio::streambuf raw_response;
	boost::system::error_code ec;

	std::string read_string;
	std::size_t bytes_read;
	while((bytes_read = boost::asio::read(*socket_, raw_response, 
							 boost::asio::transfer_at_least(1), ec))) {

		// Read the values form the buffer (all bytes read)
		std::string read_data = std::string(boost::asio::buffers_begin(raw_response.data()),
											boost::asio::buffers_begin(raw_response.data()) + bytes_read);
		
		read_string += read_data;
		// Remove the bytes read from the buffer
		raw_response.consume(bytes_read);
	}


	if(ec != boost::asio::error::eof) {
		// Error reading.
		printf("**************errored out\n");
		return nullptr;
	}

	auto resp = Response::Parse(read_string);
	return resp;

}

HTTPClient::~HTTPClient() {
	// free(socket_);
}