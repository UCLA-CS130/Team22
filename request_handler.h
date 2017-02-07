#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>
#include <list>
#include <memory>
#include "http_parser.h"

class RequestHandler {
public:
	virtual std::string GenerateResponse(const HttpParser& headerInfo, const std::string& requestData) const = 0;

	static std::string generate_error(std::string reason)
	{
		reason = "404 NOT FOUND\r\n" + reason;
		std::stringstream error_ss;
		error_ss << "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " << reason.length() << "\r\n\r\n";
		error_ss << reason;

		return error_ss.str();
	}
};

// list of k,v pairs, where k = path and v = pointer to request handler
typedef std::list<std::pair<const std::string, std::unique_ptr<RequestHandler>>> HandlerContainer;
typedef std::pair<const std::string, std::unique_ptr<RequestHandler>> HandlerPair;

/*
#include <boost/asio.hpp>
#include <string>
#include <unordered_map>
#include "connection.h"
#include "config_parser.h"

//class to handle requests
class RequestHandler
{
public:
	static std::string handle_echo(size_t bytes_transferred, char* data);

	static std::string handle_file_server(std::string file_path);

private:
	enum { max_length = 8192 };

	static std::unordered_map<std::string,std::string> content_mappings;
};
*/

#endif // REQUEST_HANDLER_H
