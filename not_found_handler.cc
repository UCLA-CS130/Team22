#include <string>

#include "request_handler.h"
#include "not_found_handler.h"
#include "response.h"
#include "config_parser.h"
#include <boost/log/trivial.hpp>

RequestHandler::Status NotFoundHandler::Init(const std::string& uri_prefix, const NginxConfig& config)
{
	return RequestHandler::OK;
}

RequestHandler::Status NotFoundHandler::HandleRequest(const Request& request, Response* response)
{
	BOOST_LOG_TRIVIAL(trace) << "Creating not found handler response";

	std::string reason = "404 NOT FOUND\r\n";

	response->SetStatus(Response::not_found);
	response->AddHeader("Content-Type", "text/html");
	response->AddHeader("Content-Length", std::to_string(reason.length()));
	response->SetBody(reason);
	return RequestHandler::OK;
}
