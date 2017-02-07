#include <string>
#include <sstream>

#include "request_handler.h"
#include "echo_handler.h"

std::string EchoHandler::GenerateResponse(const HttpParser& headerInfo, const std::string& requestData) const
{
	std::stringstream ss;
	ss << "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " << requestData.length() << "\r\n\r\n";
	ss << requestData; // the echo, includes the header

	return ss.str();
}