#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "request_handler.h"
#include "server.h"

class ServerStatus;

using boost::asio::ip::tcp;

//handles each individual connection
class Connection
{
public:
	Connection(boost::asio::io_service& io_service, const HandlerContainer* handlers, ServerStatus* serverStatus);

	tcp::socket& socket();

	void start();

	void handle_request(const boost::system::error_code& error, size_t bytes_transferred);

	std::string write_response(const Response& response);

private:
	// Close socket after sending response
	void close_socket(const boost::system::error_code& error);

	// returns a request handler if it was defined in the config, otherwise returns NULL
	const RequestHandler* GetRequestHandler(const std::string & path);
	std::string get_prefix(const std::string uri);

	tcp::socket socket_;
	enum { max_length = 8192 }; // 8KB max length
	char data_[max_length];

	const HandlerContainer* handlers_;
	ServerStatus* serverStatus_;
	std::string response_data_;
};


#endif // CONNECTION_H
