#include <string>

#include "request_handler.h"
#include "echo_handler.h"
#include "response.h"

RequestHandler::Status EchoHandler::HandleRequest(const Request& request, Response* response) const
{
	response->SetStatus(Response::ok);
	response->AddHeader("Content-Type", "text/plain");
	response->AddHeader("Content-Length", std::to_string(request.raw_request().length()));
	response->SetBody(request.raw_request());
	return RequestHandler::OK;
}