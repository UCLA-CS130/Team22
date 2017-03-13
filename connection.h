#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "request_handler.h"
#include "server.h"

class ServerStatus;

using boost::asio::ip::tcp;

// A connection is viewed as asynchronous from the view of the server
//  1. construct a disconnected connection (listening for new connections) (async)
//  2. wait for a connection - acceptor_.async_accept(new_connection->socket(),... (async)
//  3. call start() (async)

//handles each individual connection
class Connection
{
public:
	Connection(boost::asio::io_service& io_service, const HandlerContainer* handlers, ServerStatus* serverStatus);
	~Connection();

	// creates the socket
	tcp::socket& socket();

	// starts the connection and begins async_read_some
	void start();

	// handles an incoming request, gets the proper handler, and writes the response
	void handle_request(const boost::system::error_code& error, size_t bytes_transferred);

	// writes the response from Response object
	void write_response(const Response& response);

	std::string GetStatus();

private:
	// Close socket after sending response
	void close_socket(const boost::system::error_code& error);

	tcp::socket socket_;
	enum { max_length = 8192 }; // 8KB max length
	boost::asio::streambuf data_stream_;

	const HandlerContainer* handlers_;
	ServerStatus* serverStatus_;
	std::string response_data_;

	std::string request_summary_;

	enum ConnectionState {
		LISTENING,
		READING,
		PROCESSING,
		WRITING,
		CLOSING
	};
	ConnectionState conn_state_ = LISTENING;
	std::mutex conn_state_lock_;
	void SetState(ConnectionState); // utility, set state with lock
};


#endif // CONNECTION_H
