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

const std::unordered_map<std::string,std::string> content_mappings
{
	{ "gif", "image/gif" },
	{ "htm", "text/html" },
	{ "html", "text/html" },
	{ "jpg", "image/jpeg" },
	{ "jpeg", "image/jpeg" },
	{ "png", "image/png" },
	{ "md", "text/markdown" }
};

RequestHandler::Status StaticHandler::Init(const std::string& uri_prefix, const NginxConfig& config)
{
	// initialiing uri prefix and directory
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

	std::ifstream infile(file_path.c_str());
	if (infile.is_open()) {
		std::stringstream ss;
		ss << infile.rdbuf();
		std::string body_data = ss.str();

		std::string new_data;
		std::string content_type;
		if (!ProcessFile(file_path, body_data, &new_data, &content_type)){
			return RequestHandler::ERROR;
		}

		if (content_type != ""){
			response->AddHeader("Content-Type", content_type);
		}

		response->AddHeader("Content-Length", std::to_string(body_data.length()));
		response->SetBody(new_data);
		response->SetStatus(Response::ok);
	}
	else {
		BOOST_LOG_TRIVIAL(error) << "Unable to open file: " << file_path;
		NotFoundHandler not_found_handler;
		return not_found_handler.HandleRequest(request, response);
	}

	return RequestHandler::OK;
}


bool StaticHandler::ProcessFile(const std::string& path, const std::string& data, std::string *new_data, std::string *content_type) const{
	*new_data = data; // there must be a better way

	// Find the content type
	std::size_t last_dot_pos = path.find_last_of(".");
	if(last_dot_pos != std::string::npos) {
		std::string file_extension = path.substr(last_dot_pos + 1);
	
		auto it = content_mappings.find(file_extension);
		if (it != content_mappings.end()) {
			*content_type = it->second;
		}
	}
	return true;
}
