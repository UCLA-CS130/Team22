#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>
#include <list>
#include <memory>
#include "request.h"
#include "response.h"

class RequestHandler {
public:

	// requestData is the full http request
	// request is the parsed header
	virtual Response HandleRequest(const Request& request) const = 0;
};

// list of k,v pairs, where k = path and v = pointer to request handler
typedef std::list<std::pair<const std::string, std::unique_ptr<RequestHandler>>> HandlerContainer;
typedef std::pair<const std::string, std::unique_ptr<RequestHandler>> HandlerPair;

#endif // REQUEST_HANDLER_H
