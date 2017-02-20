#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>
#include <list>
#include <memory>
#include "request.h"

class RequestHandler {
public:

	// requestData is the full http request
	// request is the parsed header
	virtual std::string HandleRequest(const Request& request) const = 0;

	static std::string generate_error(std::string reason)
	{
		reason = "404 NOT FOUND\r\n" + reason;
		std::stringstream error_ss;
		error_ss << "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " << reason.length() << "\r\n\r\n";
		error_ss << reason;

		return error_ss.str();
	}
};

// list of k,v pairs, where k = path and v = pointer to request handler
typedef std::list<std::pair<const std::string, std::unique_ptr<RequestHandler>>> HandlerContainer;
typedef std::pair<const std::string, std::unique_ptr<RequestHandler>> HandlerPair;

#endif // REQUEST_HANDLER_H
