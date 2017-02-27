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
		if(statement->tokens_.size() == 2 && statement->tokens_[0] == "proxy_pass")
		{
			url_ = statement->tokens_[1];

			std::string::size_type pos = url_.find('/');
			if(pos != std::string::npos) {
				host_ = url_.substr(0, pos);
				path_ = url_.substr(pos);
			}
			else {
				host_ = url_;
				path_ = "/";
			}
			
			return RequestHandler::OK;
		}
	}

	BOOST_LOG_TRIVIAL(error) << "proxy_pass not specified in static handler for " << uri_prefix;
	return RequestHandler::ERROR;
}

RequestHandler::Status ReverseProxyHandler::HandleRequest(const Request& request, Response* response) const
{
	BOOST_LOG_TRIVIAL(trace) << "Creating Reverse Proxy Response...";

    Request OutgoingRequest = TransformIncomingRequest(request);

	// TODO: Send the outgoing request
	
    // for(int i = 0; i < MaxRedirectDepth + 1; i++) {
	// 	BOOST_LOG_TRIVIAL(trace) << "Reaching out to " << OutgoingRequest.uri();
        
    // }


	response->SetStatus(Response::ok);
	return RequestHandler::OK;
}


Request ReverseProxyHandler::TransformIncomingRequest(const Request& request) const {
    Request transformed_request(request);
	transformed_request.set_header(std::make_pair("Host", host_));
	transformed_request.set_uri(path_ + request.uri().substr(prefix_.length()));
    return transformed_request;
}