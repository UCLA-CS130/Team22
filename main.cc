#include <iostream>
#include <boost/asio.hpp>
#include "server.h"
#include "config_parser.h"
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <thread>
#include <system_error>

int main(int argc, char* argv[])
{
	try
	{
		//logging level is everything by default
		if (argc == 2)
		{
			boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
		}
		//set default logging level to everything above and including info
		else if (argc == 3)
		{
			if (std::string(argv[2]) == "-d")
				boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);
			else if (std::string(argv[2]) == "-s")
				boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::error);
			else {
				BOOST_LOG_TRIVIAL(fatal) << std::string(argv[2]) << " flag not recognized";
				return 1;
			}
		}
		//expect command to be './webserver config' or './webserver config -d'
		else
		{
			BOOST_LOG_TRIVIAL(fatal) << "Incorrect command. Usage: web_server <config file>. Add \'-d\' to the end to enable all debugging trace logs.";
			return 1;
		}

		BOOST_LOG_TRIVIAL(info) << "Starting up the server...";
		NginxConfigParser parser;
		NginxConfig out_config;
		if (!parser.Parse(argv[1], &out_config)) {
			BOOST_LOG_TRIVIAL(fatal) << "Error parsing input config file.";
			return 1;
		}

		boost::asio::io_service io_service;

		auto server = std::unique_ptr<Server>(Server::MakeServer(io_service, out_config));
		if (server.get() == nullptr)
		{
			BOOST_LOG_TRIVIAL(fatal) << "Error starting server after parsing config.";
			return 1;
		}

		// Multithreading
		// TODO: encapsulate in server?

		// assume 1 thread, if we can find a definition in the config then take it
		size_t max_threads = 1;  
		out_config.Find<size_t>("threads", &max_threads);

		if (max_threads > 128) { // beautiful arbitrary constraints
			BOOST_LOG_TRIVIAL(fatal) << "Please no more than 128 threads";
		}

		size_t numThreads;
		std::vector<std::thread> threads;
		threads.reserve(max_threads);
		for (numThreads = 0; numThreads < max_threads; numThreads++) {
			try {
				BOOST_LOG_TRIVIAL(trace) << "Creating thread: " << numThreads;
				threads.push_back(std::thread([&io_service]() { io_service.run(); }));
			}
			catch (std::system_error& e) {
				BOOST_LOG_TRIVIAL(error) << "Error creating another thread. Stopping at " << numThreads << " threads.";
				break;
			}
		}

		for (size_t i = 0; i < numThreads; i++) {
			threads[i].join(); // these joins should never happen since each is in an infinite loop
			BOOST_LOG_TRIVIAL(trace) << "Joining thread: " << i;
		}
	}
	catch (std::exception& e)
	{
		BOOST_LOG_TRIVIAL(fatal) << "Exception: " << e.what();
	}

	return 0;
}
