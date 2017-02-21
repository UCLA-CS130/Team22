#ifndef NOT_FOUND_HANDLER_H
#define NOT_FOUND_HANDLER_H

#include <string>

#include "request_handler.h"
#include "config_parser.h"
#include "request.h"
#include "response.h"

class NotFoundHandler : public RequestHandler {
public:
	RequestHandler::Status Init(const std::string& uri_prefix, const NginxConfig& config);
	NotFoundHandler(const std::string& reason);
	virtual RequestHandler::Status HandleRequest(const Request& request, Response* response) const;

private:
	std::string reason_;
};


#endif // NOT_FOUND_HANDLER_H
