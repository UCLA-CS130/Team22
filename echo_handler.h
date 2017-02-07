#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include <string>

#include "request_handler.h"
#include "config_parser.h"
#include "http_parser.h"

class EchoHandler : public RequestHandler {
public:
	
	// data is the full http request
	// headerInfo is the parsed header
	virtual std::string GenerateResponse(const HttpParser& headerInfo, const std::string& requestData);

private:
	
};


#endif // ECHO_HANDLER_H
