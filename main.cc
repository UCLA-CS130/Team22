#include <iostream>
#include <boost/asio.hpp>
#include "server.h"
#include "config_parser.h"

int main(int argc, char* argv[])
{
	try
	{
		if (argc != 2)
		{
			std::cerr << "Usage: web_server <config file>\n";
			return 1;
		}

		NginxConfigParser parser;
		NginxConfig out_config;
		if (!parser.Parse(argv[1], &out_config)) {
			std::cerr << "Error parsing input config file\n";
			return 1;
		}

		boost::asio::io_service io_service;

		auto server = std::auto_ptr<Server>(Server::MakeServer(io_service, out_config));
		if(server.get() == nullptr)
		{
			std::cerr << "invalid port\n";
			return 1;
		}

		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}


	return 0;
}
