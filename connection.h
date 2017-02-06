#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "server.h"

using boost::asio::ip::tcp;
class Server;

//handles each individual connection
class Connection
{
public:
	Connection(Server* parent, boost::asio::io_service& io_service);

	tcp::socket& socket();

	void start();

	void handle_request(const boost::system::error_code& error, size_t bytes_transferred);

	std::string handle_data_write(size_t bytes_transferred, char* data);

private:
	Server* server_;

	// Close socket after sending response
	void close_socket(const boost::system::error_code& error);

	// construct response by placing request after headers
	void copy_request(char* response, char* data, size_t bytes_transferred, size_t header_length);

	tcp::socket socket_;
	enum { max_length = 1024 };
	char data_[max_length];
};


#endif // CONNECTION_H
