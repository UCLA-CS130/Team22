#include <iostream>
#include <string>
#include <cstring>
#include "server.h"

using boost::asio::ip::tcp;

Server::Server(boost::asio::io_service& io_service, short port) : io_service_(io_service),
		acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
{
	start_accept();
}

void Server::start_accept()
{
	//create new connection for incoming request, send to handle_accept
	Connection* new_connection = new Connection(io_service_);
	acceptor_.async_accept(new_connection->socket(),
		boost::bind(&Server::handle_accept, this, new_connection,
			boost::asio::placeholders::error));
}

void Server::handle_accept(Connection* new_connection, const boost::system::error_code& error)
{
	//start the connection if no error, clean up otherwise. Go back to waiting at start_accept
	if (!error)
	{
		new_connection->start();
	}
	else
	{
		delete new_connection;
	}

	start_accept();
}
