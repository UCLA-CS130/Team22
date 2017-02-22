#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <map>
#include "server.h"
#include "config_parser.h"
#include "echo_handler.h"
#include "file_handler.h"
#include "not_found_handler.h"

using boost::asio::ip::tcp;

Server* Server::MakeServer(boost::asio::io_service& io_service, NginxConfig& out_config)
{
	// generate request handlers
	auto handlers = new HandlerContainer();
	int port = 0;

	//make sure parse succeeds in finding all relavant attributes
	bool parse_status = parse_config(out_config, port, handlers);
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

bool Server::parse_config(const NginxConfig& config, int& port, HandlerContainer* const handlers)
{
	//then look for keyword 'port' that indicates the specific port number.
	//assert that port value must be found in server{...}

	bool port_found = false;

	for (auto statement : config.statements_) {
		//catch port number OR default handler instantiations
		if (statement->tokens_.size() == 2 ) {
			//port
			if (statement->tokens_[0] == "port") {
				int parsed_port;
				try {
					parsed_port = std::stoi(statement->tokens_[1]);
				}
				catch (...) { // conversion error
					return false;
				}

				//error check that port is in bounds, break if not
				if (parsed_port <= 0 || parsed_port > 65535)
				{
					return false;
				}

				port = parsed_port;
				port_found = true;
			}

			//default handler
			else if(statement->tokens_[0] == "default" && statement->child_block_ != nullptr) {
				RequestHandler* handler = RequestHandler::CreateByName(statement->tokens_[1]);
				std::string empty_string = "";
				handler->Init(empty_string, *(statement->child_block_).get()); //default handler to use "" as uri?
				std::pair<std::map<std::string, RequestHandler*>::iterator, bool> insert_result = handlers->insert(std::make_pair(empty_string, handler));

				//default already exists
				if (!insert_result.second)
				{
					return false;
				}
			}
		}
		//generic handler instantiation
		else if (statement->tokens_.size() == 3 && statement->tokens_[0] == "path" && statement->child_block_ != nullptr) {
			RequestHandler* handler;
			if(statement->tokens_[2] == "EchoHandler")
			{
				handler = new EchoHandler();
			}
			else if(statement->tokens_[2] == "StaticHandler")
			{
				handler = new FileHandler();
			}
			else if(statement->tokens_[2] == "NotFoundHandler")
			{
				handler = new NotFoundHandler("");
			}
			else
			{
				std::cerr << "Handler not found" << std::endl;
				return false;
			}
			handler->Init(statement->tokens_[1], *(statement->child_block_).get());
			std::pair<std::map<std::string, RequestHandler*>::iterator, bool> insert_result = handlers->insert(std::make_pair(statement->tokens_[1], handler));

			//prevent duplicate uri keys
			if (!insert_result.second)
			{
				return false;
			}

		}
	}
	if(!port_found)
	{
		return false;
	}

	return true;
}
