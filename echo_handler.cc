#include <string>

#include "request_handler.h"
#include "echo_handler.h"
#include "response.h"
#include "config_parser.h"
#include <boost/log/trivial.hpp>

RequestHandler::Status EchoHandler::Init(const std::string& uri_prefix, const NginxConfig& config)
{
	return RequestHandler::OK;
}

RequestHandler::Status EchoHandler::HandleRequest(const Request& request, Response* response)
{
	BOOST_LOG_TRIVIAL(trace) << "Creating echo response...";

	response->SetStatus(Response::ok);
	response->AddHeader("Content-Type", "text/plain");
	response->AddHeader("Content-Length", std::to_string(request.raw_request().length()));
	response->SetBody(request.raw_request());
	return RequestHandler::OK;
}
