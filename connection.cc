#include <iostream>
#include <cstring>
#include "connection.h"
#include "request_handler.h"

using boost::asio::ip::tcp;

Connection::Connection(boost::asio::io_service& io_service, const HandlerContainer& handlers) : socket_(io_service), handlers_(&handlers)
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
		// parse header
		HttpParser parsedHeader = HttpParser(data_); // unsafe, is there no byte count or anything?

		// get the correct handler based on the header
		RequestHandler* handler = GetRequestHandler(parsedHeader.get_path());
		
		// have the handler generate a response
		std::string data = std::string(data_, bytes_transferred);
		std::string response = handler->GenerateResponse(parsedHeader, data);

		// write out the response
		boost::asio::async_write(
			socket_,
			boost::asio::buffer(response),
			boost::bind(&Connection::close_socket, this,
				boost::asio::placeholders::error));
	}
	else
	{
		delete this;
	}
}

std::string Connection::handle_data_write(size_t bytes_transferred, char* data)
{
	//append headers setting response and content type, and echo back in body
	char response[max_length];
	sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: %d\n\n", (int)bytes_transferred);
	size_t header_length = std::strlen(response);
	copy_request(response, data, bytes_transferred, header_length);
	//write response back
	boost::asio::async_write(
		socket_,
		boost::asio::buffer(response, bytes_transferred + header_length),
		boost::bind(&Connection::close_socket, this,
			boost::asio::placeholders::error));

	return std::string(response);
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

// construct response by placing request after headers
void Connection::copy_request(char* response, char* data, size_t bytes_transferred, size_t header_length)
{
    std::memcpy(&response[header_length], data, bytes_transferred);
}

RequestHandler* Connection::GetRequestHandler(const std::string& path)
{
	for (auto i : requestHandlers_) {


	}
}