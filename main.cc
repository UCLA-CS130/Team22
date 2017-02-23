#include <iostream>
#include <boost/asio.hpp>
#include "server.h"
#include "config_parser.h"
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

int main(int argc, char* argv[])
{
	BOOST_LOG_TRIVIAL(info) << "Starting up the server...";
	try
	{
		//expect command to be './webserver config' or './webserver config -d'
		if (argc > 3 || argc < 2)
		{
			BOOST_LOG_TRIVIAL(fatal) << "Incorrect command. Usage: web_server <config file>. Add \'-d\' to the end to enable all debugging trace logs.";
			return 1;
		}

		//logging level is everything by default
		if(argc == 3)
		{
			if(std::string(argv[2]) != "-d")
				BOOST_LOG_TRIVIAL(warning) << "flag not recognized, ignoring flag.";

		}

		//set default logging level to everything above and including info
		if(argc == 2)
			boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);

		NginxConfigParser parser;
		NginxConfig out_config;
		if (!parser.Parse(argv[1], &out_config)) {
			BOOST_LOG_TRIVIAL(fatal) << "Error parsing input config file.";
			return 1;
		}

		boost::asio::io_service io_service;

		auto server = std::auto_ptr<Server>(Server::MakeServer(io_service, out_config));
		if(server.get() == nullptr)
		{
			BOOST_LOG_TRIVIAL(fatal) << "Error starting server after parsing config.";
			return 1;
		}

		io_service.run();
	}
	catch (std::exception& e)
	{
		BOOST_LOG_TRIVIAL(fatal) << "Exception: " << e.what();
	}


	return 0;
}
