#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>
#include "request_handler.h"
#include "config_parser.h"
#include "http_parser.h"

class FileHandler : public RequestHandler {
public:
	// data is the full http request
	// headerInfo is the parsed header
	virtual std::string GenerateResponse(const HttpParser& headerInfo, const std::string& requestData) const;

private:
	enum { max_length = 8192 };
	static std::unordered_map<std::string,std::string> content_mappings;
};

#endif // FILE_HANDLER_H
