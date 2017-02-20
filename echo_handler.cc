#include <string>
#include <sstream>

#include "request_handler.h"
#include "echo_handler.h"

std::string EchoHandler::HandleRequest(const Request& request) const
{
	std::stringstream ss;
	ss << "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " << request.raw_request().length() << "\r\n\r\n";
	ss << request.raw_request(); // the echo, includes the header

	return ss.str();
}