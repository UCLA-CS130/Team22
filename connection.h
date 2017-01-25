#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

//handles each individual connection
class Connection
{
public:
	Connection(boost::asio::io_service& io_service);

	tcp::socket& socket();

	void start();

private:
	void handle_read(const boost::system::error_code& error, size_t bytes_transferred);

	// Close socket after sending response
	void close_socket(const boost::system::error_code& error);

	// construct response by placing request after headers
	void copy_request(char* response, size_t bytes_transferred, size_t header_length);

	tcp::socket socket_;
	enum { max_length = 1024 };
	char data_[max_length];
};


#endif // CONNECTION_H
