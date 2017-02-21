#ifndef STATUS_HANDLER_H
#define STATUS_HANDLER_H

#include <string>

#include "request_handler.h"
#include "config_parser.h"
#include "request.h"
#include "server_status.h"

class StatusHandler : public RequestHandler {
public:
	RequestHandler::Status Init(const std::string& uri_prefix, const NginxConfig& config);

	// This handler has special initialization because it uses shared data
	void InitStatusHandler(const HandlerContainer*, const ServerStatus*);

	RequestHandler::Status HandleRequest(const Request& request, Response* response);

private:
	const HandlerContainer* handlers_;
	const ServerStatus* serverStatus_;
};

REGISTER_REQUEST_HANDLER(StaticFileHandler);

#endif // STATUS_HANDLER_H
