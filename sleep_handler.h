#ifndef SLEEP_HANDLER_H
#define SLEEP_HANDLER_H

#include <string>

#include "request_handler.h"
#include "config_parser.h"
#include "request.h"
#include "response.h"

class SleepHandler : public RequestHandler {
public:
	virtual RequestHandler::Status Init(const std::string& uri_prefix, const NginxConfig& config);
	virtual RequestHandler::Status HandleRequest(const Request& request, Response* response) const;

private:
	int seconds_;
};

REGISTER_REQUEST_HANDLER(SleepHandler);


#endif // SLEEP_HANDLER_H
