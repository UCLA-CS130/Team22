#ifndef STATUS_HANDLER_H
#define STATUS_HANDLER_H

#include <string>

#include "request_handler.h"
#include "config_parser.h"
#include "request.h"

class StatusHandler : public RequestHandler {
public:
	virtual Status HandleRequest(const Request& request, Response* response);

private:
	const ServerStatus* serverStatus_;
};

REGISTER_REQUEST_HANDLER(StaticFileHandler);

#endif // STATUS_HANDLER_H
