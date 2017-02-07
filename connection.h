#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "request_handler.h"

using boost::asio::ip::tcp;

//handles each individual connection
class Connection
{
public:
	Connection(boost::asio::io_service& io_service, const HandlerContainer* handlers);

	tcp::socket& socket();

	void start();

	void handle_request(const boost::system::error_code& error, size_t bytes_transferred);

private:
	// Close socket after sending response
	void close_socket(const boost::system::error_code& error);

	// construct response by placing request after headers
	void copy_request(char* response, char* data, size_t bytes_transferred, size_t header_length);

	// returns a request handler if it was defined in the config, otherwise returns NULL
	const RequestHandler* GetRequestHandler(const std::string & path);

	tcp::socket socket_;
	enum { max_length = 1024 };
	char data_[max_length];

	const HandlerContainer* handlers_;
};


#endif // CONNECTION_H
