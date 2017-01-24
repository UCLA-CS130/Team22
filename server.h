#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <string>
#include "connection.h"

using boost::asio::ip::tcp;

class Server
{
public:
	Server(boost::asio::io_service& io_service, short port);

private:
	void start_accept();
	void handle_accept(Connection* new_connection, const boost::system::error_code& error);

	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
};

#endif // SERVER_H