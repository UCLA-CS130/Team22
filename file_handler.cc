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

std::string FileHandler::GenerateResponse(const HttpParser& headerInfo, const std::string& requestData) const
{
	std::stringstream ss;
	std::string file_path = "static/kinkakuji.jpg";

	std::size_t last_dot_pos = file_path.find_last_of(".");
	if(last_dot_pos == std::string::npos)
	{
		std::cerr << "Could not find extension";
		return "";
	}
	else
	{
		std::string file_extension = file_path.substr(last_dot_pos + 1);

		std::unordered_map<std::string,std::string>::const_iterator it = content_mappings.find(file_extension);
		if (it == content_mappings.end())
		{
			std::cerr << "Extension not supported";
			return "";
		}
		else
		{
			std::string content_type = it->second;

			std::string line;
			std::ifstream infile(file_path.c_str(), std::ifstream::ate | std::ifstream::binary);
			if (infile.is_open())
			{
				int filesize = infile.tellg();
				ss << "HTTP/1.1 200 OK\nContent-Type: " << content_type << "\r\n";
				ss << "Content-Length: " << std::to_string(filesize) << "\r\n\r\n";
			}
			else
			{
				std::cerr << "Unable to open file" << std::endl;
				return "";
			}

			// reset back to beginning
			infile.clear();
			infile.seekg(0, std::ios::beg);
			char buf[max_length];
			while (infile.read(buf, sizeof(buf)).gcount() > 0) {
				ss << buf;
			}
		}
	}
	return ss.str();;
}
