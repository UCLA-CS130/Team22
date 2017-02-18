#ifndef NOT_FOUND_HANDLER_H
#define NOT_FOUND_HANDLER_H

#include <string>

#include "request_handler.h"
#include "config_parser.h"
#include "http_parser.h"

class NotFoundHandler : public RequestHandler {
public:
	
	virtual std::string GenerateResponse(const HttpParser& headerInfo, const std::string& requestData) const;
	
};


#endif // NOT_FOUND_HANDLER_H
