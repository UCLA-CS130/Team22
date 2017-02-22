#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <string>
#include <list>
#include <memory>

#include "connection.h"
#include "config_parser.h"
#include "request_handler.h"

using boost::asio::ip::tcp;

//general Server class, creates and manages incoming connections
class Server
{
public:
	static Server* MakeServer(boost::asio::io_service& io_service, NginxConfig& out_config);

	struct Status {
		int port;
		// std::chrono::duration<double> uptime_seconds;
		std::map<std::string, int> requestCountByURL;
		std::map<int, int> responseCountByCode;
		std::list<std::string> requestHandlers;
		// Open connections
		// Last 10 requests
	};
	Status GetStatus();

	void LogRequest(std::string url, int responseCode);

private:
	//private constructor for Server
	Server(boost::asio::io_service& io_service, int port, HandlerContainer* handlers);
	//general function to listen for connections
	void start_accept();
	//handle creating a new connection when a request comes in
	void handle_accept(Connection* new_connection, const boost::system::error_code& error);

	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;

	std::unique_ptr<HandlerContainer> requestHandlers_;
	std::map<std::string, int> requestCountByURL_;
	std::map<int, int> responseCountByCode_;
};


#endif // SERVER_H
