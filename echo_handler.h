#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include <string>

#include "request_handler.h"
#include "config_parser.h"
#include "http_parser.h"

class EchoHandler : public RequestHandler {
public:
	
	virtual std::string GenerateResponse(const HttpParser& headerInfo, const std::string& requestData) const;
	
};


#endif // ECHO_HANDLER_H
