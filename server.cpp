#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include "server.h"
#include "config_parser.h"

using boost::asio::ip::tcp;

Server* Server::MakeServer(boost::asio::io_service& io_service, NginxConfig& out_config)
{
	//handles invalid ports by throwing to main from within the GetPort function
	int port = out_config.GetPort();
	if (port == -1) {
		return nullptr;
	}
	return new Server(io_service, port);
}

Server::Server(boost::asio::io_service& io_service, int port) : io_service_(io_service), acceptor_(io_service)
{
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
	acceptor_.listen();
	start_accept();
	std::cout << "Listening on port " << port << "..." << std::endl;
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
