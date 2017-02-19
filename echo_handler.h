#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include <string>

#include "request_handler.h"
#include "config_parser.h"
#include "request.h"

class EchoHandler : public RequestHandler {
public:
	
	virtual std::string GenerateResponse(std::unique_ptr<Request> &request) const;
	
};


#endif // ECHO_HANDLER_H
