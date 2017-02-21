#ifndef REQUEST_HANDLERS_H
#define REQUEST_HANDLERS_H

#include <string>
#include <unordered_map>
#include <map>
#include <memory>
#include "request.h"
#include "response.h"


//----------------------------------------------------
//	RequestHandler
//----------------------------------------------------

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


//----------------------------------------------------
//	EchoHandler
//----------------------------------------------------

class EchoHandler : public RequestHandler {
public:
	virtual RequestHandler::Status HandleRequest(const Request& request, Response* response) const;
};


//----------------------------------------------------
//	FileHandler
//----------------------------------------------------

class FileHandler : public RequestHandler {
public:
	FileHandler(const std::string& directory);
	// data is the full http request
	// request is the parsed request
	virtual RequestHandler::Status HandleRequest(const Request& request, Response* response) const;

private:
	enum { max_length = 8192 };
	std::string directory_;
};


//----------------------------------------------------
//	NotFoundHandler
//----------------------------------------------------

class NotFoundHandler : public RequestHandler {
public:
	NotFoundHandler(const std::string& reason);
	virtual RequestHandler::Status HandleRequest(const Request& request, Response* response) const;

private:
	std::string reason_;
};

#endif // REQUEST_HANDLERS_H
