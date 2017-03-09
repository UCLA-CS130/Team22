#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <string>
#include <list>
#include <memory>
#include <mutex>
#include <set>

#include "connection.h"
#include "config_parser.h"
#include "request_handler.h"
#include "server.h"

class ServerStatus;
class Connection;

using boost::asio::ip::tcp;

//general Server class, creates and manages incoming connections
class Server
{
public:
	static Server* MakeServer(boost::asio::io_service& io_service, NginxConfig& out_config);

private:
	//private constructor for Server
	Server(boost::asio::io_service& io_service, int port, HandlerContainer* handlers, ServerStatus* serverStatus);
	//general function to listen for connections
	void start_accept();
	//handle creating a new connection when a request comes in
	void handle_accept(Connection* new_connection, const boost::system::error_code& error);

	//parse the out_config to create handlers and find port
	static bool parse_config(const NginxConfig& config, int& port, HandlerContainer* const handlers, ServerStatus* server);

	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;

	std::unique_ptr<HandlerContainer> requestHandlers_;
	std::unique_ptr<ServerStatus> serverStatus_;
};

// a class of copiable member variables... but more like logging varaibles
class ServerStatus {
	friend class Server;
public:
	struct Snapshot {
		int port_ = 0;
		// std::chrono::duration<double> uptime_seconds;
		int totalRequests_ = 0;
		std::map<std::string, int> requestCountByURL_;
		std::map<int, int> responseCountByCode_;
		std::list<std::string> requestHandlers_;
		// Open connections
		std::list<std::string> openConnections_;
		// Last 10 requests
		
	};
	Snapshot GetSnapshot(); // returns a copy of the status
	void LogRequest(std::string url, int responseCode);

	void AddConnection(Connection*);
	void RemoveConnection(Connection*);

private:
	std::mutex sharedStateLock_;
	Snapshot sharedState_;

	std::set<Connection*> connections_;
};

#endif // SERVER_H
