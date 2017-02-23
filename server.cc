#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <map>
#include <memory>
#include <boost/log/trivial.hpp>

#include "server.h"
#include "config_parser.h"
#include "request_handler.h"
#include "echo_handler.h"
#include "static_handler.h"
#include "not_found_handler.h"

using boost::asio::ip::tcp;

Server* Server::MakeServer(boost::asio::io_service& io_service, NginxConfig& out_config)
{
	// generate request handlers
	HandlerContainer* handlers = new HandlerContainer();
	int port = 0;

	BOOST_LOG_TRIVIAL(trace) << "Scanning config for port and handlers...";
	//make sure parse succeeds in finding all relavant attributes
	bool parse_status = parse_config(out_config, port, handlers);
	if (!parse_status) {
		BOOST_LOG_TRIVIAL(fatal) << "Failed to extract keyword(s) from config.";
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
	BOOST_LOG_TRIVIAL(info) << "Server constructed, listening on port " << port << "...";
}

void Server::start_accept()
{
	//create new connection for incoming request, send to handle_accept
	Connection* new_connection = new Connection(io_service_, requestHandlers_.get());
	BOOST_LOG_TRIVIAL(trace) << "New connection created...";
	acceptor_.async_accept(new_connection->socket(),
		boost::bind(&Server::handle_accept, this, new_connection,
			boost::asio::placeholders::error));
}

void Server::handle_accept(Connection* new_connection, const boost::system::error_code& error)
{
	//start the connection if no error, clean up otherwise. Go back to waiting at start_accept
	if (!error)
	{
		BOOST_LOG_TRIVIAL(trace) << "starting new connection...";
		new_connection->start();
	}
	else
	{
		BOOST_LOG_TRIVIAL(error) << "New connection failed to start.";
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
					BOOST_LOG_TRIVIAL(fatal) << "Port number in config is not an integer.";
					return false;
				}

				//error check that port is in bounds, break if not
				if (parsed_port <= 0 || parsed_port > 65535)
				{
					BOOST_LOG_TRIVIAL(fatal) << "Port number in not in range [1, 65535].";
					return false;
				}

				port = parsed_port;
				BOOST_LOG_TRIVIAL(trace) << "Port found as " << port;

				port_found = true;
			}

			//default handler
			else if(statement->tokens_[0] == "default" && statement->child_block_ != nullptr) {
				RequestHandler* handler = RequestHandler::CreateByName(statement->tokens_[1]);
				std::string empty_string = "";
				if(handler->Init(empty_string, *(statement->child_block_).get()) == RequestHandler::ERROR)
				{
					BOOST_LOG_TRIVIAL(fatal) << "Error with RequestHandler Init for default";
					return false;
				}
				std::pair<std::map<std::string, RequestHandler*>::iterator, bool> insert_result = handlers->insert(std::make_pair(empty_string, handler));
				//default already exists
				if (!insert_result.second)
				{
					BOOST_LOG_TRIVIAL(fatal) << "Duplicate handler uri detected.";
					return false;
				}

				BOOST_LOG_TRIVIAL(trace) << "Default handler found, called " << statement->tokens_[1];
			}
		}
		//generic handler instantiation
		else if (statement->tokens_.size() == 3 && statement->tokens_[0] == "path" && statement->child_block_ != nullptr) {
			RequestHandler* handler = RequestHandler::CreateByName(statement->tokens_[2]);

			if(handler->Init(statement->tokens_[1], *(statement->child_block_).get()) == RequestHandler::ERROR)
			{
				BOOST_LOG_TRIVIAL(fatal) << "Error with RequestHandler Init for " << statement->tokens_[1];
				return false;
			}
			std::pair<std::map<std::string, RequestHandler*>::iterator, bool> insert_result = handlers->insert(std::make_pair(statement->tokens_[1], handler));

			//prevent duplicate uri keys
			if (!insert_result.second)
			{
				BOOST_LOG_TRIVIAL(fatal) << "Duplicate handler uri detected.";
				return false;
			}

			BOOST_LOG_TRIVIAL(info) << "Handler found defining uri " << statement->tokens_[1] << " to " << statement->tokens_[2];

		}
	}
	if(!port_found)
	{
		BOOST_LOG_TRIVIAL(fatal) << "Port number failed to be parsed from config.";
		return false;
	}

	return true;
}
