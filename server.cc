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
#include "status_handler.h"

using boost::asio::ip::tcp;

Server* Server::MakeServer(boost::asio::io_service& io_service, NginxConfig& out_config)
{
	// generate request handlers
	HandlerContainer* handlers = new HandlerContainer();
	int port = 0;
	ServerStatus* serverStatus = new ServerStatus();

	BOOST_LOG_TRIVIAL(trace) << "Scanning config for port and handlers...";
	//make sure parse succeeds in finding all relavant attributes
	bool parse_status = parse_config(out_config, port, handlers, serverStatus);
	if (!parse_status) {
		BOOST_LOG_TRIVIAL(fatal) << "Failed to extract keyword(s) from config.";
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

	// initialize ServerStatus
	serverStatus_->sharedState_.port_ = port;
	serverStatus_->sharedState_.totalRequests_ = 0;
	for (auto& handlerPair : *requestHandlers_) {
		serverStatus_->sharedState_.requestHandlers_.push_back(handlerPair.first);
	}
	BOOST_LOG_TRIVIAL(info) << "Server constructed, listening on port " << port << "...";
}

void Server::start_accept()
{
	//create new connection for incoming request, send to handle_accept
	Connection* new_connection = new Connection(io_service_, requestHandlers_.get(), serverStatus_.get());
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

			// Special case initialization
			if (statement->tokens_[2] == "StatusHandler") {
				StatusHandler* statusHandler = dynamic_cast<StatusHandler*>(handler);
				statusHandler->InitStatusHandler(serverStatus);
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

void ServerStatus::AddConnection(Connection* conn)
{
	connections_.insert(conn);
}
void ServerStatus::RemoveConnection(Connection* conn)
{
	connections_.erase(conn);
}

void ServerStatus::LogRequest(std::string url, int responseCode)
{
	// multiple connections will be touching this, so we should lock it
	std::lock_guard<std::mutex> lock(sharedStateLock_);

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
	Snapshot snapshot;
	{
		// copy the existing state
		std::lock_guard<std::mutex> lock(sharedStateLock_);
		snapshot = sharedState_; 
	}

	// some extra dynamic info
	for (auto i : connections_) {
		snapshot.openConnections_.push_back(i->GetStatus());
	}
	
	return snapshot;
}

