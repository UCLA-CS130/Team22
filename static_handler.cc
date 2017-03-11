#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "markdown.h"
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
	{ "png", "image/png" },
	{ "md", "text/html" }
};


RequestHandler::Status StaticHandler::Init(const std::string& uri_prefix, const NginxConfig& config)
{
	// initializing variables
	prefix_ = uri_prefix;
	timeout_ = -1;

	for (auto statement : config.statements_)
	{
		//look for root
		if(statement->tokens_.size() == 2 && statement->tokens_[0] == "root")
		{
			directory_ = statement->tokens_[1];
		}

		//look for authetication list for private files (optional)
		else if(statement->tokens_.size() == 2 && statement->tokens_[0] == "authentication_list")
		{
			if(!init_authentication_database(statement->tokens_[1]))
			{
				BOOST_LOG_TRIVIAL(error) << "error parsing given authentication_list file";
				return RequestHandler::ERROR;
			}
		}

		//look for timeout value in seconds for authentication (optional)
		else if(statement->tokens_.size() == 2 && statement->tokens_[0] == "timeout")
		{
			//exception will be thrown and caught from main if value isnt an int
			timeout_ = std::stoi(statement->tokens_[1]);
		}
	}

	//root not set
	if(directory_ == "")
	{
		BOOST_LOG_TRIVIAL(error) << "root not specified in static handler for " << uri_prefix;
		return RequestHandler::ERROR;
	}

	return RequestHandler::OK;
}


RequestHandler::Status StaticHandler::HandleRequest(const Request& request, Response* response) const
{
	BOOST_LOG_TRIVIAL(trace) << "Creating static file response";

	std::string full_path = request.uri();
	std::string file_path = directory_ + full_path.substr(prefix_.length()); // get file path relative to server

	// Make sure extension exists
	std::size_t last_dot_pos = file_path.find_last_of(".");
	if(last_dot_pos == std::string::npos)
	{
		BOOST_LOG_TRIVIAL(error) << "Unknown file: " << file_path;

		NotFoundHandler not_found_handler;
		return not_found_handler.HandleRequest(request, response);
	}
	else
	{
		// Check for how to serve extension
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
			// Set content type and find content length, then read file and place in response
			std::string content_type = it->second;

			std::ifstream infile(file_path.c_str());
			if (infile.is_open())
			{
				response->SetStatus(Response::ok);
				response->AddHeader("Content-Type", content_type);

				std::string body_data = "";

				// Convert to html if in markdown
				if(file_extension == "md")
				{
					std::ostringstream out;
					markdown::Document doc;
					doc.read(infile);
					doc.write(out);
					body_data = out.str();
				}
				else
				{
					char buf[max_length];
					while (infile.read(buf, sizeof(buf)).gcount() > 0) {
						body_data.append(buf, infile.gcount());
					}
				}

				response->AddHeader("Content-Length", std::to_string(body_data.length()));
				response->SetBody(body_data);
			}
			else
			{
				BOOST_LOG_TRIVIAL(error) << "Unable to open file: " << file_path;
				NotFoundHandler not_found_handler;
				return not_found_handler.HandleRequest(request, response);
			}
		}
	}

	return RequestHandler::OK;
}

bool StaticHandler::init_authentication_database(std::string file_name)
{
	std::string line;
	std::ifstream file(file_name);

	while(std::getline(file, line))
	{
		size_t index_of_comma = line.find(",");
		if (index_of_comma == std::string::npos)
		{
			BOOST_LOG_TRIVIAL(error) << "Comma missing in authentication file";
			return false;
		}

		std::string username = line.substr(0, index_of_comma);
		std::string password = line.substr(index_of_comma+1);

		//check if username already exists
		if(authentication_map_.find(username) != authentication_map_.end())
		{
			BOOST_LOG_TRIVIAL(error) << "Unallowed duplicate usernames exist";
			return false;
		}
		else
		{
			authentication_map_[username] = password;
		}
	}

	return true;
}
