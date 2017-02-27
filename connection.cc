#include <iostream>
#include <cstring>
#include <memory>
#include <fstream>
#include "connection.h"
#include "request_handler.h"
#include "not_found_handler.h"
#include "request.h"
#include "response.h"
#include <boost/log/trivial.hpp>

using boost::asio::ip::tcp;

Connection::Connection(boost::asio::io_service& io_service, const HandlerContainer* handlers, ServerStatus* serverStatus) : socket_(io_service), handlers_(handlers), serverStatus_(serverStatus)
{
}

// creates the socket
tcp::socket& Connection::socket()
{
	return socket_;
}

// starts the connection and begins async_read_some
void Connection::start()
{
	BOOST_LOG_TRIVIAL(trace) << "======connection started==========";
	socket_.async_read_some(boost::asio::buffer(data_, max_length),
		boost::bind(&Connection::handle_request, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

// handles an incoming request, gets the proper handler, and writes the response
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
			// TODO generalize, fit with the StaticHandler
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

		if (serverStatus_) 
			serverStatus_->LogRequest(request->uri(), response.GetStatusCode());
	}
	else
	{
		delete this;
	}
}

// writes the response from Response object
// returns the data that was written
std::string Connection::write_response(const Response& response)
{
	BOOST_LOG_TRIVIAL(trace) << "Writing response...";

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
		BOOST_LOG_TRIVIAL(trace) << "======conection closed===========";
	} else {
		BOOST_LOG_TRIVIAL(error) << "error closing connection.";
		delete this;
	}
}

// returns a request handler if it was defined in the config, otherwise returns nullptr
const RequestHandler* Connection::GetRequestHandler(const std::string& path)
{
	//exact match
	std::map<const std::string, RequestHandler*>::const_iterator match = handlers_->find(path);
	if(match != handlers_->end())
		return match->second;

	//longest prefix match
	std::string prefix = get_prefix(path);
	match = handlers_->find(prefix);
	if(match != handlers_->end())
		return match->second;

	return nullptr;
}

// gets longest matching prefix
std::string Connection::get_prefix(const std::string uri)
{
	std::string longest = "";
	size_t index = uri.find_last_of("/");
	std::string prefix = uri.substr(0, index);

	for (auto& handlerPair : *handlers_)
	{
		std::string uri_key = handlerPair.first;

		//uri_key is within this prefix
		if(prefix.find(uri_key) == 0 && uri_key.length() > longest.length())
			longest = uri_key;
	}

	return longest;
}
