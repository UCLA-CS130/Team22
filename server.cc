#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include "server.h"
#include "config_parser.h"
#include "request_handlers.h"

using boost::asio::ip::tcp;

Server* Server::MakeServer(boost::asio::io_service& io_service, NginxConfig& out_config)
{
	//make sure parse succeeds in finding all relavant attributes
	bool parse_status = out_config.ParseStatements();
	if (!parse_status) {
		return nullptr;
	}

	int port = out_config.GetPort();
	
	// generate request handlers
	HandlerContainer *handlers = new HandlerContainer();

	// Populate echo paths
	std::shared_ptr<std::vector<std::string>> echo_paths = out_config.GetEchoPaths();
	for(auto echo_path : *echo_paths)
	{
		handlers->insert(std::make_pair(echo_path, std::unique_ptr<RequestHandler>(new EchoHandler())));
	}

	// Populate file server paths
	std::shared_ptr<std::map<std::string, std::string>> file_paths = out_config.GetFilePaths();
	for(auto file_path : *file_paths)
	{
		handlers->insert(std::make_pair(file_path.first, std::unique_ptr<RequestHandler>(new FileHandler(file_path.second))));
	}

	return new Server(io_service, port, handlers);
}

Server::Server(boost::asio::io_service& io_service, int port, HandlerContainer* handlers) : io_service_(io_service), acceptor_(io_service), requestHandlers_(handlers)
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
	Connection* new_connection = new Connection(io_service_, requestHandlers_.get());
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
