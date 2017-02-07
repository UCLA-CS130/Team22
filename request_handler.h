#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>
#include <list>
#include <memory>
#include "http_parser.h"

class RequestHandler {
public:
	virtual std::string GenerateResponse(const HttpParser& headerInfo, const std::string& requestData) const = 0;

};

// list of k,v pairs, where k = path and v = pointer to request handler
typedef std::list<std::pair<const std::string, std::unique_ptr<RequestHandler>>> HandlerContainer;
typedef std::pair<const std::string, std::unique_ptr<RequestHandler>> HandlerPair;

#endif // REQUEST_HANDLER_H
