#include "status_handler.h"
#include <sstream>

void StatusHandler::InitStatusHandler(const HandlerContainer* handlers, const ServerStatus* serverStatus)
{
	handlers_ = handlers;
	serverStatus_ = serverStatus;
}

RequestHandler::Status StatusHandler::HandleRequest(const Request & request, Response * response)
{
	if (handlers_ == NULL || serverStatus_ == NULL) {
		return RequestHandler::ERROR;
	}

	auto responseCountList = serverStatus->ResponseCountByStatus();
	int responseTotal = serverStatus->TotalResponses();

	std::stringstream body;

	// html stuff
	body << "<!DOCTYPE HTML>\n";
	body << "<html>\n";
	body << "<h1>Webserver Status Page</h1>\n";

	// Print a list of response counts
	body << "<h3>Response Counts</h3>\n";
	body << "<ul style = \"list-style-type:none\">\n";
	for (auto responseCountPair : responseCountList) {
		body << "<li>" << responseCountPair.first << " : " << responseCountPair.second << "</li>\n";
	}
	body << "<li>total : " <<  << "</li>\n";
	body << "</ul>\n";

	// Print a list of handlers
	body << "<h3>Handlers</h3>\n";
	body << "<ul style = \"list-style-type:none\">\n";
	for (auto& handlerPair : *handlers_) {
		body << "<li>" << handlerPair.first << "</li>\n";
	}
	body << "</ul>\n";
	body << "</html>\n";

	response->SetStatus(Response::ok);
	response->AddHeader("Content-Type", "text/plain");
	response->AddHeader("Content-Length", std::to_string(request.raw_request().length()));	response->SetBody(body.str());
	return RequestHandler::OK;
}
