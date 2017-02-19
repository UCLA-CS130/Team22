#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>
#include <memory>
#include <unordered_map>
#include "request_handler.h"
#include "config_parser.h"
#include "request.h"

class FileHandler : public RequestHandler {
public:
	FileHandler(std::string directory);
	// data is the full http request
	// request is the parsed request
	virtual std::string GenerateResponse(std::unique_ptr<Request> &request) const;

private:
	enum { max_length = 8192 };
	static std::unordered_map<std::string,std::string> content_mappings;
	std::string directory_;
};

#endif // FILE_HANDLER_H
