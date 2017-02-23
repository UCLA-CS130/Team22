#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>
#include <unordered_map>
#include "request_handler.h"
#include "config_parser.h"
#include "request.h"
#include "response.h"

class StaticHandler : public RequestHandler {
public:
	RequestHandler::Status Init(const std::string& uri_prefix, const NginxConfig& config);
	virtual RequestHandler::Status HandleRequest(const Request& request, Response* response) const;

private:
	enum { max_length = 8192 };
	std::string directory_;
};

REGISTER_REQUEST_HANDLER(StaticHandler);

#endif // FILE_HANDLER_H
