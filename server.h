#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <string>
#include <list>
#include <memory>

#include "connection.h"
#include "config_parser.h"
#include "request_handler.h"

using boost::asio::ip::tcp;

//general Server class, creates and manages incoming connections
class Server
{
public:
	static Server* MakeServer(boost::asio::io_service& io_service, NginxConfig& out_config);

private:
	//private constructor for Server
	Server(boost::asio::io_service& io_service, int port, HandlerContainer* handlers);
	//general function to listen for connections
	void start_accept();
	//handle creating a new connection when a request comes in
	void handle_accept(Connection* new_connection, const boost::system::error_code& error);

	//parse the out_config to create handlers and find port
	bool parse_config(const NginxConfig* const config, int* const port, HandlerContainer* const handlers);

	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;

	std::unique_ptr<HandlerContainer> requestHandlers_;
};

#endif // SERVER_H
