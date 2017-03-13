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
//#include "echo_handler.h"
//#include "static_handler.h"
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

	serverStatus_->sharedState_.requestHandlers_ = handlers->GetList();

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
				if(!handler){
					BOOST_LOG_TRIVIAL(fatal) << "Invalid handler type: " << statement->tokens_[3];
					return false;
				}
				std::string empty_string = "";
				if(handler->Init(empty_string, *(statement->child_block_).get()) == RequestHandler::ERROR)
				{
					BOOST_LOG_TRIVIAL(fatal) << "Error with RequestHandler Init for default";
					return false;
				}

				if (!handlers->AddPath("", handler)){
					BOOST_LOG_TRIVIAL(fatal) << "Duplicate default handler detected.";
					return false;
				}

				BOOST_LOG_TRIVIAL(trace) << "Default handler found, called " << statement->tokens_[1];
			}
		}
		//generic handler instantiation, path
		else if (statement->tokens_.size() == 3 && 
				statement->tokens_[0] == "path" &&
				statement->child_block_ != nullptr) {
			RequestHandler* handler = RequestHandler::CreateByName(statement->tokens_[2]);
			if(!handler){
				BOOST_LOG_TRIVIAL(fatal) << "Invalid handler type: " << statement->tokens_[3];
				return false;
			}

			if(handler->Init(statement->tokens_[1], *(statement->child_block_).get()) == RequestHandler::ERROR)
			{
				BOOST_LOG_TRIVIAL(fatal) << "Error with RequestHandler Init for " << statement->tokens_[1];
				return false;
			}

			if (!handlers->AddPath(statement->tokens_[1], handler)){
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
		//generic handler instantiation, path_regex
		else if (statement->tokens_.size() == 4 && 
				statement->tokens_[0] == "path_regex" &&
				statement->child_block_ != nullptr) {
			RequestHandler* handler = RequestHandler::CreateByName(statement->tokens_[3]);
			if(!handler){
				BOOST_LOG_TRIVIAL(fatal) << "Invalid handler type: " << statement->tokens_[3];
				return false;
			}

			if(handler->Init(statement->tokens_[1], *(statement->child_block_).get()) == RequestHandler::ERROR)
			{
				BOOST_LOG_TRIVIAL(fatal) << "Error with RequestHandler Init for " << statement->tokens_[1];
				return false;
			}

			if (!handlers->AddRegexPath(statement->tokens_[1], statement->tokens_[2], handler)){
				BOOST_LOG_TRIVIAL(fatal) << "Error creating handler " << statement->tokens_[1] << " " << statement->tokens_[2];
				return false;
			}

			if (statement->tokens_[3] == "StatusHandler") {
				StatusHandler* statusHandler = dynamic_cast<StatusHandler*>(handler);
				statusHandler->InitStatusHandler(serverStatus);
			}
			BOOST_LOG_TRIVIAL(info) << "Handler found defining uri " << statement->tokens_[1] << " " << statement->tokens_[2] << " to " << statement->tokens_[3];
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

void ServerStatus::LogRequest(std::string prefix, std::string url, int responseCode)
{
	// multiple connections will be touching this, so we should lock it
	std::lock_guard<std::mutex> lock(sharedStateLock_);

	// count by code
	//   200 : 10
	//   404 : 3
	// if doesn't exist insert a 1
	// pair<iterator,bool> insertPair
	auto insertPair = sharedState_.responseCountByCode_.insert(std::make_pair(responseCode, 1));

	// if already exists, increment
	if (insertPair.second == false) {
		// insertPair first is a pair<code,count> iterator
		std::map<int, int>::iterator it = insertPair.first;
		it->second++;
	}

	// count by prefix and url
	//   proxy 1 :
	//     /cat.jpg : 4
	//     /index.html : 10
	//   proxy 2:
	//     /dog.png : 3
	auto prefixInsertPair = sharedState_.requestCountByURL_.insert(
		std::make_pair(prefix, std::map<std::string, int>({{url,1}}))); // insert a single {{url, 1}} if prefix not found
	if (prefixInsertPair.second == false) {
		auto urlInsertPair = prefixInsertPair.first->second.insert(std::make_pair(url, 1)); // insert a {url, 1} if url not found
		if (urlInsertPair.second == false) {
			urlInsertPair.first->second++; // increment the existing counter
		}
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

