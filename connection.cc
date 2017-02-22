#include <iostream>
#include <cstring>
#include <memory>
#include <fstream>
#include "connection.h"
#include "request_handler.h"
#include "not_found_handler.h"
#include "request.h"
#include "response.h"

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
		Response response;

		// parse header
		std::string data = std::string(data_, bytes_transferred);

		auto request = Request::Parse(data);

		// get the correct handler based on the header
		const RequestHandler* handler = GetRequestHandler(request->uri());
		
		if (handler == nullptr) {
			// TODO generalize, fit with the StaticFileHandler
			NotFoundHandler not_found_handler;
			not_found_handler.HandleRequest(*request, &response);
		}
		else {
			// have the handler generate a response
			RequestHandler::Status status = handler->HandleRequest(*request, &response);
			if(status != RequestHandler::OK)
			{
				response = Response();
				response.SetStatus(Response::internal_server_error);
				response.SetBody("500 Internal Server Error");
			}
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

std::string Connection::write_response(const Response& response)
{
	response_data_ = response.ToString();
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
	// for each k,v pair
	for (auto& handlerPair : *handlers_) {
		std::size_t second_slash_pos = path.find("/", 1);
		std::string search_path = path.substr(0, second_slash_pos);

		// check if handler key (/echo) is at the beginning of the path
		if (search_path.compare(handlerPair.first) == 0) {
			// return the handler pointer
			return handlerPair.second.get();
		}
	}

	return nullptr;
}
