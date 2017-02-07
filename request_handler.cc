#include <iostream>
#include <cstring>
#include <fstream>
#include <unordered_map>
#include "connection.h"
#include "request_handler.h"

using boost::asio::ip::tcp;

std::unordered_map<std::string,std::string> RequestHandler::content_mappings =
{
	{ "gif", "image/gif" },
	{ "htm", "text/html" },
	{ "html", "text/html" },
	{ "jpg", "image/jpeg" },
	{ "jpeg", "image/jpeg" },
	{ "png", "image/png" }
};

std::string RequestHandler::handle_echo(size_t bytes_transferred, char* data)
{
	//append headers setting response and content type, and echo back in body
	std::string response_data = "";
	response_data += "HTTP/1.1 200 OK\nContent-Type: text/plain\n";
	response_data += "Content-Length: ";
	response_data += std::to_string(bytes_transferred);
	response_data += "\n\n";
	response_data += data;
	return response_data;
}

std::string RequestHandler::handle_file_server(std::string file_path)
{
	std::string response_data = "";

	std::size_t last_dot_pos = file_path.find_last_of(".");
	if(last_dot_pos == std::string::npos)
	{
		std::cerr << "Could not find extension";
	}
	else
	{
		std::string file_extension = file_path.substr(last_dot_pos + 1);

		std::unordered_map<std::string,std::string>::const_iterator it = content_mappings.find(file_extension);
		if (it == content_mappings.end())
		{
			std::cerr << "Extension not supported";
		}
		else
		{
			std::string content_type = it->second;

			std::string line;
			std::ifstream infile(file_path.c_str(), std::ifstream::ate | std::ifstream::binary);
			if (infile.is_open())
			{
				int filesize = infile.tellg();
				response_data += "HTTP/1.1 200 OK\nContent-Type: ";
				response_data += content_type;
				response_data += "\r\n";
				response_data += "Content-Length: ";
				response_data += std::to_string(filesize);
				response_data += "\r\n\r\n";
			}
			else
			{
				std::cerr << "Unable to open file" << std::endl;
				return response_data;
			}

			// reset back to beginning
			infile.clear();
			infile.seekg(0, std::ios::beg);
			char buf[max_length];
			while (infile.read(buf, sizeof(buf)).gcount() > 0) {
				response_data.append(buf, infile.gcount());
			}
		}
	}
	

	return response_data;
}