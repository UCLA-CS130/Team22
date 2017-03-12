#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>
#include <map>
#include <list>
#include <memory>
#include "request.h"
#include "response.h"
#include "config_parser.h"

class RequestHandler {
public:
	enum Status {
		OK = 0,
		ERROR = 1
	};

	// Initializes the handler. Returns a response code indicating success or
	// failure condition.
	// uri_prefix is the value in the config file that this handler will run for.
	// config is the contents of the child block for this handler ONLY.
	virtual Status Init(const std::string& uri_prefix, const NginxConfig& config) = 0;

	// Creation Magic by name of handler as specified in config
	static RequestHandler* CreateByName(const std::string& type);

	// Handles an HTTP request, and generates a response. Returns a response code
	// indicating success or failure condition. If ResponseCode is not OK, the
	// contents of the response object are undefined, and the server will return
	// HTTP code 500.
	virtual RequestHandler::Status HandleRequest(const Request& request, Response* response) const = 0;
};

// Notes:
// * The trick here is that you can declare an object at file scope, but you
//   can't do anything else, such as set a map key. But you can get around this
//   by creating a class that does work in its constructor.
// * request_handler_builders must be a pointer. Otherwise, it won't necessarily
//   exist when the RequestHandlerRegisterer constructor gets called.

extern std::map<std::string, RequestHandler* (*)(void)>* request_handler_builders;
template<typename T>
class RequestHandlerRegisterer {
public:
	RequestHandlerRegisterer(const std::string& type) {
		if (request_handler_builders == nullptr) {
			request_handler_builders = new std::map<std::string, RequestHandler* (*)(void)>;
		}
		(*request_handler_builders)[type] = RequestHandlerRegisterer::Create;
	}
	static RequestHandler* Create() {
		return new T;
	}
};

#define REGISTER_REQUEST_HANDLER(ClassName) static RequestHandlerRegisterer<ClassName> ClassName##__registerer(#ClassName)


// Enapsulates prefix path searching, regex searching
// Hides the internal storage structure for handlers
// Calling Add gives ownership to this container
class HandlerContainer {
public:
	bool AddPath(const std::string& prefix, const RequestHandler* handler);
	bool AddRegexPath(const std::string& prefix, const std::string& regex, const RequestHandler* handler);

	// returns a request handler if it was defined in the config, otherwise returns nullptr
	const RequestHandler* Find(const std::string& path) const;

	// returns a list of request handler paths (for the status page)
	std::list<std::string> GetList() const;

private:
	// gets longest matching prefix
	std::string get_prefix(const std::string& uri) const;

	std::map<const std::string, std::unique_ptr<const RequestHandler>> paths_;
	std::map<const std::string, std::pair<const std::string, std::unique_ptr<const RequestHandler>>> regex_paths_;
};


#endif // REQUEST_HANDLER_H
