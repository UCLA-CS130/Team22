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
    int port;
    if (!parser.Parse(argv[1], &out_config)) {
      return 1;
    }
    port = out_config.GetPort();
    if (port == -1) {
      std::cerr << "Error reading port from config file. Expecting: \"port [portnumber];\" in config file.";
      return 1;
    }

		boost::asio::io_service io_service;

    Server s(io_service, port);

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}