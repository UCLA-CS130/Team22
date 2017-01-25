//
// Server Adapted from Boost Example: async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2016 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

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

		Server s(io_service, out_config);

		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
