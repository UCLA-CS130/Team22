#ifndef MARKDOWN_HANDLER_H
#define MARKDOWN_HANDLER_H

#include <string>
#include <unordered_map>
#include "request_handler.h"
#include "config_parser.h"
#include "request.h"
#include "response.h"

class MarkdownHandler : public RequestHandler {
public:
	RequestHandler::Status Init(const std::string& uri_prefix, const NginxConfig& config);
	virtual RequestHandler::Status HandleRequest(const Request& request, Response* response) const;

private:
	// directory path local to server
	std::string file_;

	// prefix of uri
	std::string prefix_;
};

REGISTER_REQUEST_HANDLER(MarkdownHandler);

#endif // MARKDOWN_HANDLER_H
