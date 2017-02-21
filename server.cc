#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include "server.h"
#include "config_parser.h"
#include "echo_handler.h"
#include "file_handler.h"

using boost::asio::ip::tcp;

Server* Server::MakeServer(boost::asio::io_service& io_service, NginxConfig& out_config)
{
	// generate request handlers
	HandlerContainer *handlers = new HandlerContainer();
	int port = 0;

	//make sure parse succeeds in finding all relavant attributes
	bool parse_status = out_config.parse_config(&port, handlers);
	if (!parse_status) {
		return nullptr;
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

bool parse_config(int* const port, HandlerContainer* const handlers)
{
	//then look for keyword 'port' that indicates the specific port number.
	//assert that port value must be found in server{...}
	bool port_found = false;

	for (auto statement : statements_) {
		//catch port number OR default handler instantiations
		if (statement->tokens_.size() == 2 ) {
			//port
			if (statement->tokens_[0] == "port") {
				int parsed_port;
				try {
					parsed_port = std::stoi(statement->tokens_[1]);
					*port = parsed_port;
				}
				catch (...) { // conversion error
					return false;
				}

				//error check that port is in bounds, break if not
				if (port <= 0 || port > 65535)
					return false;
				port_ = port;
				port_found = true;
			}

			//default handler
			else if(statement->tokens_[0] == "default" && statement->child_block_ != nullptr) {
				default_handler->handler_name = statement->tokens_[1];
				default_handler->child_block = statement->child_block_;
			}
		}
		//generic handler instantiation
		else if (statement->tokens_.size() == 3 && statement->tokens_[0] == "path" && statement->child_block_ != nullptr) {
			std::string uri = statement->tokens_[1];
			std::shared_ptr<handler_options> options = std::make_shared<handler_options>();

			options->handler_name = statement->tokens_[2];
			options->child_block = statement->child_block_;

			//prevent duplicate uri keys
			if (handlers_map->find(uri) != handlers_map->end())
				return false;

			handlers_map->insert(std::make_pair(uri, options));
		}
	}
	if(!port_found)
		return false;

	return true;
}
