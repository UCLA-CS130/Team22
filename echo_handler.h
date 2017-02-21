#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include <string>

#include "request_handler.h"
#include "config_parser.h"
#include "request.h"
#include "response.h"

class EchoHandler : public RequestHandler {
public:
	virtual RequestHandler::Status HandleRequest(const Request& request, Response* response) const;
	
};


#endif // ECHO_HANDLER_H
