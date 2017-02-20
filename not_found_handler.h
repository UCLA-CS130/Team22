#ifndef NOT_FOUND_HANDLER_H
#define NOT_FOUND_HANDLER_H

#include <string>

#include "request_handler.h"
#include "config_parser.h"
#include "request.h"
#include "response.h"

class NotFoundHandler : public RequestHandler {
public:
	NotFoundHandler(const std::string& reason);
	virtual Response HandleRequest(const Request& request) const;

private:
	std::string reason_;
};


#endif // NOT_FOUND_HANDLER_H
