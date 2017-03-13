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

Connection::Connection(boost::asio::io_service& io_service, const HandlerContainer* handlers, ServerStatus* serverStatus)
	: socket_(io_service)
	, data_stream_(max_length)
	, handlers_(handlers)
	, serverStatus_(serverStatus)
	, conn_state_(LISTENING)
{
	if (serverStatus) serverStatus->AddConnection(this);
}
Connection::~Connection(){
	if (serverStatus_) serverStatus_->RemoveConnection(this);
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

	SetState(READING);

	boost::asio::async_read_until(socket_, data_stream_, "\r\n\r\n",
		boost::bind(&Connection::handle_request, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

// handles an incoming request, gets the proper handler, and writes the response
void Connection::handle_request(const boost::system::error_code& error, size_t bytes_transferred)
{
	BOOST_LOG_TRIVIAL(trace) << "Finished reading...";
	SetState(PROCESSING);

	if (error == boost::asio::error::eof){
		BOOST_LOG_TRIVIAL(trace) << "EOF received";
		close_socket(boost::system::errc::make_error_code(boost::system::errc::success));
		return;
	}
	if (error == boost::asio::error::not_found){
		BOOST_LOG_TRIVIAL(warning) << "Received a very long request";
		close_socket(boost::system::errc::make_error_code(boost::system::errc::success));
		return;
	}
	if (!error) // no error
	{
		Response response;

		std::string data((std::istreambuf_iterator<char>(&data_stream_)), std::istreambuf_iterator<char>());

		std::string prefix = "unknown";
		std::unique_ptr<Request> request = Request::Parse(data);
		if (!request) { // parse error -> nullptr
			response.SetStatus(Response::bad_request);
			response.SetBody("400 Bad Request\r\n");
			prefix = "Bad Request";
		}
		else {
			bool post_request_error = false;
			
			// read in the rest of a POST request
			if (request->method() == "POST") {
				std::string content_length_string = request->get_header("Content-Length");
				if(content_length_string == "")
				{
					response.SetStatus(Response::bad_request);
					response.SetBody("400 Bad Request");
					post_request_error = true;
				}
				else
				{
					size_t content_length = std::stoi(content_length_string);

					char buff[1000];
					size_t body_size = request->body().size();
					while (body_size < content_length) {
						int nread;
						try {
							nread = socket_.read_some(boost::asio::buffer(buff, 1000));
						}
						catch (...){
							// client closed the connection
							// would close_socket() work?
							return;
						}
						body_size = request->append_body(std::string(buff, nread)); // append the body and raw_request, update
					}
				}
			}

			if(!post_request_error) {
				// log what we're up to
				request_summary_ = request->uri();

				// get the correct handler based on the header
				RequestHandler* handler = GetRequestHandler(request->uri(), &prefix);

				if (handler == nullptr) {
					// TODO generalize, fit with the StaticHandler
					NotFoundHandler not_found_handler;
					not_found_handler.HandleRequest(*request, &response);
					prefix = "Not Found";
				}
				else {
					// have the handler generate a response
					RequestHandler::Status status = handler->HandleRequest(*request, &response);
					if (status != RequestHandler::OK)
					{
						response.SetStatus(Response::internal_server_error);
						response.SetBody("500 Internal Server Error");
					}
				}
			}
		}

		// write out the response
		write_response(response);

		if (serverStatus_) {
			std::string uri = request ? request->uri() : "invalid";
			serverStatus_->LogRequest(prefix, uri, response.GetStatusCode());
		}
	}
	else
	{
		BOOST_LOG_TRIVIAL(error) << "async_read_until failed - " << error.message();
		close_socket(boost::system::errc::make_error_code(boost::system::errc::success));
	}
}

// writes the response from Response object
// returns the data that was written
std::string Connection::write_response(const Response& response)
{
	BOOST_LOG_TRIVIAL(trace) << "Writing response...";
	SetState(WRITING);

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
	SetState(CLOSING);
	if (!error) {
		// we don't actually need these
		//socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
		//socket_.close();
		BOOST_LOG_TRIVIAL(trace) << "Finished writing";
	} else {
		BOOST_LOG_TRIVIAL(error) << "Error writing connection.";
	}

	delete this;
	BOOST_LOG_TRIVIAL(trace) << "======conection closed===========";
}

// returns a request handler if it was defined in the config, otherwise returns nullptr
RequestHandler* Connection::GetRequestHandler(const std::string& path, std::string* pref)
{
	//exact match
	std::map<const std::string, RequestHandler*>::const_iterator match = handlers_->find(path);
	if(match != handlers_->end()){
		if (pref) *pref = match->first;
		return match->second;
	}

	//longest prefix match
	std::string prefix = get_prefix(path);
	match = handlers_->find(prefix);
	if(match != handlers_->end()){
		if (pref) *pref = match->first;
		return match->second;
	}

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

std::string Connection::GetStatus()
{
	std::stringstream status;
	ConnectionState state;
	{
		std::lock_guard<std::mutex> lock(conn_state_lock_);
		state = conn_state_;
	}

	if (state == LISTENING) {
		status << "listening for new connections";
	}
	else {
		std::string clientIP = "";
		unsigned short clientPort = 0;

		try {
			clientIP = socket_.remote_endpoint().address().to_string();
			clientPort = socket_.remote_endpoint().port();
		}
		catch (...){
			BOOST_LOG_TRIVIAL(error) << "error getting port stuff in connection GetStatus";
		}

		status << request_summary_ << ", client IP: " << clientIP << ", client port: " << clientPort << ", ";
		switch (state) {
		case READING:
			status << "reading from the socket";
			break;
		case PROCESSING:
			status << "processing";
			break;
		case WRITING:
			status << "writing to the socket";
			break;
		case CLOSING:
			status << "closing";
			break;
		default:
			break;
		}
	}

	return status.str();
}

void Connection::SetState(ConnectionState s)
{
	std::lock_guard<std::mutex> lock(conn_state_lock_);
	conn_state_ = s;
}
