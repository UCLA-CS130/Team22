#include <string>
#include <sstream>

#include "request_handler.h"
#include "not_found_handler.h"

std::string NotFoundHandler::HandleRequest(const Request& request) const
{
	std::string reason = "404 NOT FOUND\r\n";
	std::stringstream ss;
	ss << "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " << reason.length() << "\r\n\r\n";
	ss << reason;

	return ss.str();
}