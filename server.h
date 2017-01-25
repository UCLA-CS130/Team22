#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <string>
#include "connection.h"
#include "config_parser.h"

using boost::asio::ip::tcp;

//general Server class, creates and manages incoming connections
class Server
{
public:
	Server(boost::asio::io_service& io_service, NginxConfig& out_config);

private:
	//general function to listen for connections
	void start_accept();
	//handle creating a new connection when a request comes in
	void handle_accept(Connection* new_connection, const boost::system::error_code& error);

	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
};

#endif // SERVER_H
