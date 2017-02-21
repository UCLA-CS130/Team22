#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>
#include <map>
#include <memory>
#include "request.h"
#include "response.h"

class RequestHandler {
public:
	enum Status {
		OK = 0,
		ERROR = 1
	};

	// requestData is the full http request
	// request is the parsed header
	virtual RequestHandler::Status HandleRequest(const Request& request, Response* response) const = 0;
};

// map of k,v pairs, where k = path and v = pointer to request handler
typedef std::map<const std::string, std::unique_ptr<RequestHandler>> HandlerContainer;

#endif // REQUEST_HANDLER_H
