#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include <string>

#include "request_handler.h"
#include "config_parser.h"
#include "request.h"
#include "response.h"

class EchoHandler : public RequestHandler {
public:
	
	virtual Response HandleRequest(const Request& request) const;
	
};


#endif // ECHO_HANDLER_H
