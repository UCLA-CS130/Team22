#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

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

#endif // REQUEST_HANDLER_H
