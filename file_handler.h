#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>
#include <unordered_map>
#include "request_handler.h"
#include "config_parser.h"
#include "request.h"
#include "response.h"

class FileHandler : public RequestHandler {
public:
	FileHandler(const std::string& directory);
	// data is the full http request
	// request is the parsed request
	virtual RequestHandler::Status HandleRequest(const Request& request, Response* response) const;

private:
	enum { max_length = 8192 };
	static std::unordered_map<std::string,std::string> content_mappings;
	std::string directory_;
};

#endif // FILE_HANDLER_H
