#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <map>
#include <memory>
#include "server.h"
#include "config_parser.h"
#include "echo_handler.h"
#include "static_handler.h"
#include "not_found_handler.h"
#include "status_handler.h"


using boost::asio::ip::tcp;

Server* Server::MakeServer(boost::asio::io_service& io_service, NginxConfig& out_config)
{
	// generate request handlers
	HandlerContainer* handlers = new HandlerContainer();
	int port = 0;
	ServerStatus* serverStatus = new ServerStatus();

	//make sure parse succeeds in finding all relavant attributes
	bool parse_status = parse_config(out_config, port, handlers, serverStatus);
	if (!parse_status) {
		return nullptr;

	}

	return new Server(io_service, port, handlers, serverStatus);
}

Server::Server(boost::asio::io_service& io_service, int port, HandlerContainer* handlers, ServerStatus* serverStatus) : io_service_(io_service), acceptor_(io_service), requestHandlers_(handlers), serverStatus_(serverStatus)
{
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
	acceptor_.listen();
	start_accept();
	std::cout << "Listening on port " << port << "..." << std::endl;

	// initialize ServerStatus
	serverStatus_->sharedState_.port_ = port;
	serverStatus_->sharedState_.totalRequests_ = 0;
	for (auto& handlerPair : *requestHandlers_) {
		serverStatus_->sharedState_.requestHandlers_.push_back(handlerPair.first);
	}
}

void Server::start_accept()
{
	//create new connection for incoming request, send to handle_accept
	Connection* new_connection = new Connection(io_service_, requestHandlers_.get(), serverStatus_.get());
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

bool Server::parse_config(const NginxConfig& config, int& port, HandlerContainer* const handlers, ServerStatus* serverStatus)
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
			RequestHandler* handler = RequestHandler::CreateByName(statement->tokens_[2]);

			handler->Init(statement->tokens_[1], *(statement->child_block_).get());
			std::pair<std::map<std::string, RequestHandler*>::iterator, bool> insert_result = handlers->insert(std::make_pair(statement->tokens_[1], handler));

			//prevent duplicate uri keys
			if (!insert_result.second)
			{
				return false;
			}

			// Special case initialization
			if (statement->tokens_[2] == "StatusHandler") {
				StatusHandler* statusHandler = dynamic_cast<StatusHandler*>(handler);
				statusHandler->InitStatusHandler(serverStatus);
			}

		}
	}
	if(!port_found)
	{
		return false;
	}

	return true;
}

void ServerStatus::LogRequest(std::string url, int responseCode)
{
	// if doesn't exist insert a 1
	// pair<iterator,bool> insertPair
	auto insertPair = sharedState_.responseCountByCode_.insert(std::make_pair(responseCode, 1));

	// if already exists, increment
	if (insertPair.second == false) {
		// insertPair first is a pair<code,count> iterator
		std::map<int, int>::iterator it = insertPair.first;
		it->second++;
	}

	// the same thing with std::string url
	auto insertPair2 = sharedState_.requestCountByURL_.insert(std::make_pair(url, 1));
	if (insertPair2.second == false) {
		auto it = insertPair2.first;
		it->second++;
	}

	sharedState_.totalRequests_++;
}
ServerStatus::Snapshot ServerStatus::GetSnapshot()
{
	return sharedState_;
}

