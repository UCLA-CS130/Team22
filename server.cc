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

void Server::LogRequest(std::string url, int responseCode)
{
	// if doesn't exist insert a 1
	// pair<iterator,bool> insertPair
	auto insertPair = responseCountByCode_.insert(std::make_pair(responseCode, 1));

	// if already exists, increment
	if (insertPair.second == false) {
		// insertPair first is a pair<code,count> iterator
		std::map<int, int>::iterator it = insertPair.first;
		it->second++;
	}

	// the same thing with std::string url
	auto insertPair2 = requestCountByURL_.insert(std::make_pair(url, 1));
	if (insertPair2.second == false) {
		auto it = insertPair2.first;
		it->second++;
	}
}


Server::Status Server::GetStatus()
{
	Status status;
	status.port = acceptor_.local_endpoint().port();
	status.requestCountByURL = requestCountByURL_;
	status.responseCountByCode = responseCountByCode_;
	for (auto& handlerPair : *requestHandlers_) {
		status.requestHandlers.push_back(handlerPair.first);
	}

	return status;
}

