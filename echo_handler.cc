#include <string>

#include "request_handler.h"
#include "echo_handler.h"
#include "response.h"

Response EchoHandler::HandleRequest(const Request& request) const
{
	Response response;
	response.SetStatus(Response::ok);
	response.AddHeader("Content-Type", "text/plain");
	response.AddHeader("Content-Length", std::to_string(request.raw_request().length()));
	response.SetBody(request.raw_request());
	return response;
}