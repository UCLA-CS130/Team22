#include <iostream>
#include <string>
#include <fstream>

#include "request_handler.h"
#include "file_handler.h"
#include "response.h"
#include "not_found_handler.h"
#include "config_parser.h"


std::unordered_map<std::string,std::string> content_mappings
{
	{ "gif", "image/gif" },
	{ "htm", "text/html" },
	{ "html", "text/html" },
	{ "jpg", "image/jpeg" },
	{ "jpeg", "image/jpeg" },
	{ "png", "image/png" }
};


RequestHandler::Status FileHandler::Init(const std::string& uri_prefix, const NginxConfig& config)
{
	for (auto statement : config.statements_) 
	{
		if(statement->tokens_.size() == 2 && statement->tokens_[0] == "root")
		{
			directory_ = statement->tokens_[1];
			return RequestHandler::OK;
		}
	}
	return RequestHandler::ERROR;
}


RequestHandler::Status FileHandler::HandleRequest(const Request& request, Response* response) const
{
	std::string full_path = request.uri();
	std::size_t second_slash_pos = full_path.find("/", 1);
	std::string file_path = directory_ + full_path.substr(second_slash_pos + 1);

	std::size_t last_dot_pos = file_path.find_last_of(".");
	if(last_dot_pos == std::string::npos)
	{
		std::cerr << "Unknown File" << std::endl;
		NotFoundHandler not_found_handler;
		return not_found_handler.HandleRequest(request, response);
	}
	else
	{
		std::string file_extension = file_path.substr(last_dot_pos + 1);

		std::unordered_map<std::string,std::string>::const_iterator it = content_mappings.find(file_extension);
		if (it == content_mappings.end())
		{
			std::cerr << "Extension not supported" << std::endl;
			NotFoundHandler not_found_handler;
			return not_found_handler.HandleRequest(request, response);
		}
		else
		{
			std::string content_type = it->second;

			std::string line;
			std::ifstream infile(file_path.c_str(), std::ifstream::ate | std::ifstream::binary);
			if (infile.is_open())
			{
				int filesize = infile.tellg();
				response->SetStatus(Response::ok);
				response->AddHeader("Content-Type", content_type);
				response->AddHeader("Content-Length", std::to_string(filesize));
			}
			else
			{
				std::cerr << "Unable to open file" << std::endl;
				NotFoundHandler not_found_handler;
				return not_found_handler.HandleRequest(request, response);
			}

			// reset back to beginning
			infile.clear();
			infile.seekg(0, std::ios::beg);

			std::string body_data = "";
			char buf[max_length];
			while (infile.read(buf, sizeof(buf)).gcount() > 0) {
				body_data.append(buf, infile.gcount());
			}
			response->SetBody(body_data);
		}
	}
	
	return RequestHandler::OK;
}