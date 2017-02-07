#include <iostream>
#include <cstring>
#include <fstream>
#include "connection.h"
#include "request_handler.h"

using boost::asio::ip::tcp;

Connection::Connection(boost::asio::io_service& io_service) : socket_(io_service)
{
}

tcp::socket& Connection::socket()
{
	return socket_;
}

void Connection::start()
{
	socket_.async_read_some(boost::asio::buffer(data_, max_length),
		boost::bind(&Connection::handle_request, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void Connection::handle_request(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (!error)
	{
		// file server
		std::string data = RequestHandler::handle_file_server();

		// echo server
		//std::string data = RequestHandler::handle_echo(bytes_transferred, data_);

		write_response(data);
	}
	else
	{
		delete this;
	}
}

void Connection::write_response(std::string data)
{
	response_data_ = data;
	boost::asio::async_write(
		socket_,
		boost::asio::buffer(response_data_, response_data_.length()),
		boost::bind(&Connection::close_socket, this,
			boost::asio::placeholders::error));

}

// Close socket after sending response
void Connection::close_socket(const boost::system::error_code& error)
{
	if (!error) {
      socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
      socket_.close();
  } else {
      delete this;
  }
}
