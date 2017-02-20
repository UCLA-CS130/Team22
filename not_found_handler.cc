#include <string>

#include "request_handler.h"
#include "not_found_handler.h"
#include "response.h"

// Constructor to have reason
NotFoundHandler::NotFoundHandler(const std::string& reason) : reason_(reason) {}

Response NotFoundHandler::HandleRequest(const Request& request) const
{
	std::string reason = "404 NOT FOUND\r\n" + reason_;
	
	Response response;
	response.SetStatus(Response::not_found);
	response.AddHeader("Content-Type", "text/html");
	response.AddHeader("Content-Length", std::to_string(reason.length()));
	response.SetBody(reason);
	return response;
}