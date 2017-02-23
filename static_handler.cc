#include <iostream>
#include <string>
#include <fstream>

#include "request_handler.h"
#include "static_handler.h"
#include "response.h"
#include "not_found_handler.h"
#include "config_parser.h"
#include <boost/log/trivial.hpp>

std::unordered_map<std::string,std::string> content_mappings
{
	{ "gif", "image/gif" },
	{ "htm", "text/html" },
	{ "html", "text/html" },
	{ "jpg", "image/jpeg" },
	{ "jpeg", "image/jpeg" },
	{ "png", "image/png" }
};


RequestHandler::Status StaticHandler::Init(const std::string& uri_prefix, const NginxConfig& config)
{
	prefix_ = uri_prefix;

	for (auto statement : config.statements_)
	{
		if(statement->tokens_.size() == 2 && statement->tokens_[0] == "root")
		{
			directory_ = statement->tokens_[1];
			return RequestHandler::OK;
		}
	}

	BOOST_LOG_TRIVIAL(error) << "root not specified in static handler for " << uri_prefix;
	return RequestHandler::ERROR;
}


RequestHandler::Status StaticHandler::HandleRequest(const Request& request, Response* response) const
{
	BOOST_LOG_TRIVIAL(trace) << "Creating static file response";

	std::string full_path = request.uri();
	std::string file_path = directory_ + full_path.substr(prefix_.length()); // get file path relative to server

	std::size_t last_dot_pos = file_path.find_last_of(".");
	if(last_dot_pos == std::string::npos)
	{
		BOOST_LOG_TRIVIAL(error) << "Unknown file: " << file_path;

		NotFoundHandler not_found_handler;
		return not_found_handler.HandleRequest(request, response);
	}
	else
	{
		std::string file_extension = file_path.substr(last_dot_pos + 1);

		std::unordered_map<std::string,std::string>::const_iterator it = content_mappings.find(file_extension);
		if (it == content_mappings.end())
		{
			BOOST_LOG_TRIVIAL(error) << "Extension not supported: " << file_extension;
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
				BOOST_LOG_TRIVIAL(error) << "Unable to open file: " << file_path;
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
