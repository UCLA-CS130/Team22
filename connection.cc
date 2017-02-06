#include <iostream>
#include <cstring>
#include <fstream>
#include "connection.h"

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
		//handle_data_server();

		// echo server
		handle_data_write(bytes_transferred, data_);
	}
	else
	{
		delete this;
	}
}

std::string Connection::handle_data_write(size_t bytes_transferred, char* data)
{
	//append headers setting response and content type, and echo back in body
	response_data = "";
	response_data += "HTTP/1.1 200 OK\nContent-Type: text/plain\n";
	response_data += "Content-Length: ";
	response_data += std::to_string(bytes_transferred);
	response_data += "\n\n";
	response_data += data;
	//write response back
	boost::asio::async_write(
		socket_,
		boost::asio::buffer(response_data, response_data.length()),
		boost::bind(&Connection::close_socket, this,
			boost::asio::placeholders::error));

	return response_data;
}

std::string Connection::handle_data_server()
{
	//append headers setting response and content type, and echo back in body
	char content_type[] = "image/jpeg";
	char filepath[] = "static/zura.jpg";

	response_data = "";
	std::string line;
	std::ifstream infile(filepath, std::ifstream::ate | std::ifstream::binary);
	if (infile.is_open())
	{
		int filesize = infile.tellg();
		response_data += "HTTP/1.1 200 OK\nContent-Type: ";
		response_data += content_type;
		response_data += "\r\n";
		response_data += "Content-Length: ";
		response_data += std::to_string(filesize);
		response_data += "\r\n\r\n";
	}
	else
	{
		std::cerr << "Unable to open file" << std::endl;
	}

	// reset back to beginning
	infile.clear();
	infile.seekg(0, std::ios::beg);
	char buf[max_length];
    while (infile.read(buf, sizeof(buf)).gcount() > 0) {
        response_data.append(buf, infile.gcount());
        
    }
	//write response back
	boost::asio::async_write(
		socket_,
		boost::asio::buffer(response_data, response_data.length()),
		boost::bind(&Connection::close_socket, this,
			boost::asio::placeholders::error));

	return response_data;
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