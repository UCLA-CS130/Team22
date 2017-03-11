#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "markdown.h"
#include "request_handler.h"
#include "markdown_handler.h"
#include "response.h"
#include "not_found_handler.h"
#include "config_parser.h"
#include <boost/log/trivial.hpp>


RequestHandler::Status MarkdownHandler::Init(const std::string& uri_prefix, const NginxConfig& config)
{
	// initialiing uri prefix and directory
	prefix_ = uri_prefix;

	for (auto statement : config.statements_)
	{
		if(statement->tokens_.size() == 2 && statement->tokens_[0] == "file")
		{
			file_ = statement->tokens_[1];
			return RequestHandler::OK;
		}
	}

	BOOST_LOG_TRIVIAL(error) << "file not specified in markdown handler for " << uri_prefix;
	return RequestHandler::ERROR;
}


RequestHandler::Status MarkdownHandler::HandleRequest(const Request& request, Response* response) const
{
	BOOST_LOG_TRIVIAL(trace) << "Creating markdown response";

	std::ifstream infile(file_.c_str());
	std::ostringstream out;
	if (infile.is_open())
	{
		markdown::Document doc;
		doc.read(infile);
		doc.write(out);

	    std::string response_string = out.str();

		response->SetStatus(Response::ok);
		response->AddHeader("Content-Type", "text/html");
		response->AddHeader("Content-Length", std::to_string(response_string.length()));
		response->SetBody(response_string);
	}
	else
	{
		BOOST_LOG_TRIVIAL(error) << "Unable to open file: " << file_;
		NotFoundHandler not_found_handler;
		return not_found_handler.HandleRequest(request, response);
	}

	return RequestHandler::OK;
}
