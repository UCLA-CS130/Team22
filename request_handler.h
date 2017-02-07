#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>
#include <map>
#include "http_parser.h"

typedef std::map<std::unique_ptr<RequestHandler>(RequestHandler)> HandlerContainer;

class RequestHandler {
public:
	virtual std::string GenerateResponse(const HttpParser& headerInfo, const std::string& requestData) = 0;

};


#endif // REQUEST_HANDLER_H
