#ifndef NOT_FOUND_HANDLER_H
#define NOT_FOUND_HANDLER_H

#include <string>

#include "request_handler.h"
#include "config_parser.h"
#include "request.h"

class NotFoundHandler : public RequestHandler {
public:
	
	virtual std::string GenerateResponse(std::unique_ptr<Request> &request) const;
	
};


#endif // NOT_FOUND_HANDLER_H
