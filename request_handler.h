#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>
#include <map>
#include "http_parser.h"

// list of k,v pairs, where k = path and v = pointer to request handler
typedef std::list<std::pair<const std::string, std::unique_ptr<RequestHandler>>> HandlerContainer;

class RequestHandler {
public:
	virtual std::string GenerateResponse(const HttpParser& headerInfo, const std::string& requestData) = 0;

};


#endif // REQUEST_HANDLER_H
