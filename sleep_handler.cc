#include <string>
#include <chrono>
#include <thread>
#include <boost/log/trivial.hpp>

#include "sleep_handler.h"
#include "response.h"
#include "config_parser.h"

RequestHandler::Status SleepHandler::Init(const std::string& uri_prefix, const NginxConfig& config)
{
	if (!config.Find<int>("seconds", &seconds_)) {
		BOOST_LOG_TRIVIAL(error) << "seconds not specified in sleep handler for " << uri_prefix;
		return RequestHandler::ERROR;
	}

	return RequestHandler::OK;
}

RequestHandler::Status SleepHandler::HandleRequest(const Request& request, Response* response)
{
	BOOST_LOG_TRIVIAL(trace) << "Creating a sleeper..." << seconds_;

	std::this_thread::sleep_for(std::chrono::seconds(seconds_));

	std::string body = "good morning!\n";

	response->SetStatus(Response::ok);
	response->AddHeader("Content-Type", "text/plain");
	response->AddHeader("Content-Length", std::to_string(body.length()));
	response->SetBody(body);

	return RequestHandler::OK;
}
