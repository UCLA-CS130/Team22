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

	void handle_request(const boost::system::error_code& error, size_t bytes_transferred);

	std::string handle_data_write(size_t bytes_transferred, char* data);

	std::string handle_data_server();

private:

	// Close socket after sending response
	void close_socket(const boost::system::error_code& error);

	tcp::socket socket_;
	enum { max_length = 1024 };
	char data_[max_length];
	
	std::string response_data;
};


#endif // CONNECTION_H
