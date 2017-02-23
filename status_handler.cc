#include "status_handler.h"
#include <sstream>
#include <string>

RequestHandler::Status StatusHandler::Init(const std::string & uri_prefix, const NginxConfig & config)
{
	return RequestHandler::OK;
}

void StatusHandler::InitStatusHandler(Server* server)
{
	server_ = server;
}

RequestHandler::Status StatusHandler::HandleRequest(const Request & request, Response * response) const
{
	if (server_ == NULL) {
		return RequestHandler::ERROR;
	}

	Server::Status status = server_->GetStatus();

	std::string html = StatusToHtml(status);

	response->SetStatus(Response::ok);
	response->AddHeader("Content-Type", "text/html");
	response->AddHeader("Content-Length", std::to_string(html.length()));
	response->SetBody(html);

	return RequestHandler::OK;
}

std::string StatusHandler::StatusToHtml(const Server::Status& status)
{
	std::stringstream body;

	// html stuff
	body << "<!DOCTYPE HTML>\n";

	body << "<html>\n";
	body << "<title>Status Page</title>\n";
	body << "<h1>Webserver Status Page</h1>\n";

	body << "<h3>Webserver listening on port: " << status.port << "</h3>\n";

	body << "<h3>Total requests: " << status.totalRequests << "</h3>\n";

	body << "<h3>URL Request Counts</h3>\n";
	HttpPrintMap(body, status.requestCountByURL);

	body << "<h3>Response Code Counts</h3>\n";
	HttpPrintMap(body, status.responseCountByCode);

	// Print a list of handlers
	body << "<h3>Handlers</h3>\n";
	body << "<ul style = \"list-style-type:none\">\n";
	for (auto& handlerName : status.requestHandlers) {
		body << "<li>" << handlerName << "</li>\n";
	}
	body << "</ul>\n";
	body << "</html>\n";

	return body.str(); // is there a better way?
}
