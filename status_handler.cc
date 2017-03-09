#include "status_handler.h"
#include <sstream>
#include <string>

RequestHandler::Status StatusHandler::Init(const std::string & uri_prefix, const NginxConfig & config)
{
	return RequestHandler::OK;
}

void StatusHandler::InitStatusHandler(ServerStatus* serverStatus)
{
	serverStatus_ = serverStatus;
}

RequestHandler::Status StatusHandler::HandleRequest(const Request & request, Response * response) const
{
	if (serverStatus_ == NULL) {
		return RequestHandler::ERROR;
	}

	ServerStatus::Snapshot status = serverStatus_->GetSnapshot();

	std::string html = StatusToHtml(status);

	response->SetStatus(Response::ok);
	response->AddHeader("Content-Type", "text/html");
	response->AddHeader("Content-Length", std::to_string(html.length()));
	response->SetBody(html);

	return RequestHandler::OK;
}

std::string StatusHandler::StatusToHtml(const ServerStatus::Snapshot& status)
{
	std::stringstream body;

	// html stuff
	body << "<!DOCTYPE HTML>\n";

	body << "<html>\n";
	body << "<title>Status Page</title>\n";
	body << "<h1>Webserver Status Page</h1>\n";

	body << "<h3>Webserver listening on port: " << status.port_ << "</h3>\n";

	body << "<h3>Total requests: " << status.totalRequests_ << "</h3>\n";

	body << "<h3>Open Connections</h3>\n";
	HttpPrintList(body, status.openConnections_);

	body << "<h3>URL Request Counts</h3>\n";
	HttpPrintMap(body, status.requestCountByURL_);

	body << "<h3>Response Code Counts</h3>\n";
	HttpPrintMap(body, status.responseCountByCode_);

	// Print a list of handlers
	body << "<h3>Handlers</h3>\n";
	HttpPrintList(body, status.requestHandlers_);

	body << "</html>\n";

	return body.str(); // is there a better way?
}
