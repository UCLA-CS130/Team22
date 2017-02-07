#include <string>
#include <sstream>
#include <fstream>

#include "request_handler.h"
#include "file_handler.h"


std::unordered_map<std::string,std::string> FileHandler::content_mappings =
{
	{ "gif", "image/gif" },
	{ "htm", "text/html" },
	{ "html", "text/html" },
	{ "jpg", "image/jpeg" },
	{ "jpeg", "image/jpeg" },
	{ "png", "image/png" }
};

// Constructor to have directory
FileHandler::FileHandler(std::string directory) : directory_(directory) {}

std::string FileHandler::GenerateResponse(const HttpParser& headerInfo, const std::string& requestData) const
{	
	std::string response_data = "";

	std::string full_path = headerInfo.get_path();
	std::size_t second_slash_pos = full_path.find("/", 1);
	std::string file_path = directory_ + full_path.substr(second_slash_pos + 1);

	std::size_t last_dot_pos = file_path.find_last_of(".");
	if(last_dot_pos == std::string::npos)
	{
		return generate_error("Unknown File");
	}
	else
	{
		std::string file_extension = file_path.substr(last_dot_pos + 1);

		std::unordered_map<std::string,std::string>::const_iterator it = content_mappings.find(file_extension);
		if (it == content_mappings.end())
		{
			return generate_error("Extension not supported");
		}
		else
		{
			std::string content_type = it->second;

			std::string line;
			std::ifstream infile(file_path.c_str(), std::ifstream::ate | std::ifstream::binary);
			if (infile.is_open())
			{
				int filesize = infile.tellg();
				response_data += "HTTP/1.1 200 OK\r\nContent-Type: ";
				response_data += content_type;
				response_data += "\r\n";
				response_data += "Content-Length: ";
				response_data += std::to_string(filesize);
				response_data += "\r\n\r\n";
			}
			else
			{
				return generate_error("Unable to open file");
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