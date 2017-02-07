#include <iostream>
#include <cstring>
#include <memory>
#include <fstream>
#include "connection.h"
#include "request_handler.h"
#include "http_parser.h"

using boost::asio::ip::tcp;

Connection::Connection(boost::asio::io_service& io_service, const HandlerContainer* handlers) : socket_(io_service), handlers_(handlers)
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
		std::string response;

		// parse header
		auto parsedHeader = std::auto_ptr<HttpParser>(HttpParser::MakeHttpParser(data_)); // unsafe, is there no byte count or anything?

		// get the correct handler based on the header
		const RequestHandler* handler = GetRequestHandler(parsedHeader->get_path());
		
		if (handler == NULL) {
			// TODO generalize, fit with the StaticFileHandler
			response = "HTTP / 1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
		}
		else {
			// have the handler generate a response
			std::string data = std::string(data_, bytes_transferred);
			response = handler->GenerateResponse(*parsedHeader, data);
		}

		// write out the response
		write_response(response);

		/*
		// file server
		//std::string data = RequestHandler::handle_file_server("static/kinkakuji.jpg");

		// echo server
		std::string data = RequestHandler::handle_echo(bytes_transferred, data_);

		write_response(data);
		*/
	}
	else
	{
		delete this;
	}
}

std::string Connection::write_response(std::string data)
{
	response_data_ = data;
	boost::asio::async_write(
		socket_,
		boost::asio::buffer(response_data_, response_data_.length()),
		boost::bind(&Connection::close_socket, this,
			boost::asio::placeholders::error));

	return response_data_;
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

const RequestHandler* Connection::GetRequestHandler(const std::string& path)
{
	for (auto& handlerPair : *handlers_) {
		// check if handler key (/echo) is at the beginning of the path
		if (path.find(handlerPair.first) == 0) {
			// return the handler pointer
			return handlerPair.second.get();
		}
	}

	return NULL;
}
