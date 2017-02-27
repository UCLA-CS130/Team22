#include <string>
#include <memory>

#include "request_handler.h"
#include "reverse_proxy_handler.h"
#include "response.h"
#include "config_parser.h"
#include <boost/log/trivial.hpp>

RequestHandler::Status ReverseProxyHandler::Init(const std::string& uri_prefix, const NginxConfig& config)
{
	prefix_ = uri_prefix;

	for (auto statement : config.statements_)
	{
		if(statement->tokens_.size() == 2 && statement->tokens_[0] == "serve")
		{
			url_ = statement->tokens_[1];
			return RequestHandler::OK;
		}
	}

	BOOST_LOG_TRIVIAL(error) << "serve not specified in static handler for " << uri_prefix;
	return RequestHandler::ERROR;
}

RequestHandler::Status ReverseProxyHandler::HandleRequest(const Request& request, Response* response) const
{
	BOOST_LOG_TRIVIAL(trace) << "Creating Reverse Proxy Response...";

    Request OutgoingRequest = TransformRequest(request);

    for(int i = 0; i < MaxRedirectDepth; i++) {
        
    }










    //

	response->SetStatus(Response::ok);
	response->AddHeader("Content-Type", "text/plain");
	response->AddHeader("Content-Length", std::to_string(request.raw_request().length()));
	response->SetBody("potatoes");
	return RequestHandler::OK;
}


Request ReverseProxyHandler::TransformRequest(const Request& request) const {
    Request transformed_request(request);
	transformed_request.set_header();

    return transformed_request;
}