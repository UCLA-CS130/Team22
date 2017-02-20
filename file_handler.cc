#include <string>
#include <fstream>

#include "request_handler.h"
#include "file_handler.h"
#include "response.h"
#include "not_found_handler.h"


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
FileHandler::FileHandler(const std::string& directory) : directory_(directory) {}

Response FileHandler::HandleRequest(const Request& request) const
{	
	Response response;

	std::string full_path = request.uri();
	std::size_t second_slash_pos = full_path.find("/", 1);
	std::string file_path = directory_ + full_path.substr(second_slash_pos + 1);

	std::size_t last_dot_pos = file_path.find_last_of(".");
	if(last_dot_pos == std::string::npos)
	{
		NotFoundHandler not_found_handler("Unknown File");
		return not_found_handler.HandleRequest(request);
	}
	else
	{
		std::string file_extension = file_path.substr(last_dot_pos + 1);

		std::unordered_map<std::string,std::string>::const_iterator it = content_mappings.find(file_extension);
		if (it == content_mappings.end())
		{
			NotFoundHandler not_found_handler("Extension not supported");
			return not_found_handler.HandleRequest(request);
		}
		else
		{
			std::string content_type = it->second;

			std::string line;
			std::ifstream infile(file_path.c_str(), std::ifstream::ate | std::ifstream::binary);
			if (infile.is_open())
			{
				int filesize = infile.tellg();
				response.SetStatus(Response::ok);
				response.AddHeader("Content-Type", content_type);
				response.AddHeader("Content-Length", std::to_string(filesize));
			}
			else
			{
				NotFoundHandler not_found_handler("Unable to open file");
				return not_found_handler.HandleRequest(request);
			}

			// reset back to beginning
			infile.clear();
			infile.seekg(0, std::ios::beg);

			std::string body_data = "";
			char buf[max_length];
			while (infile.read(buf, sizeof(buf)).gcount() > 0) {
				body_data.append(buf, infile.gcount());
			}
			response.SetBody(body_data);
		}
	}
	
	return response;
}