#include <string>
#include <memory>

#include "request_handler.h"
#include "reverse_proxy_handler.h"
#include "response.h"
#include "config_parser.h"
#include "http_client.h"

#include <boost/log/trivial.hpp>

RequestHandler::Status ReverseProxyHandler::Init(const std::string& uri_prefix, const NginxConfig& config)
{
	prefix_ = uri_prefix;

	for (auto statement : config.statements_)
	{
		if(statement->tokens_.size() == 2 && statement->tokens_[0] == "proxy_pass")
		{
			url_ = statement->tokens_[1];

			std::string::size_type protocol_pos = url_.find("//");
			if(protocol_pos == std::string::npos) {
				BOOST_LOG_TRIVIAL(error) << "proxy_pass didn't specify protocol.'";
				return RequestHandler::ERROR;
			}
			protocol_ = url_.substr(0, protocol_pos - 1);

			std::string::size_type host_pos = url_.find('/', protocol_pos + 2);
			if(host_pos != std::string::npos) {
				host_ = url_.substr(protocol_pos + 2, host_pos - protocol_pos - 2);
				path_ = url_.substr(host_pos);
			}
			else {
				host_ = url_.substr(protocol_pos + 2);
				path_ = "/";
			}
			printf("path_: %s\n", path_.c_str());
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
	
	std::unique_ptr<Response> resp;
	
	std::string new_host = host_;
	std::string new_uri = path_;
	for(int i = 0; i < MaxRedirectDepth; i++) {
		BOOST_LOG_TRIVIAL(trace) << "Reaching out to " << new_host << " with URI: " << new_uri;
		resp.reset(); // we don't need the old response
		resp = VisitOutsideServer(OutgoingRequest, new_host, protocol_);
		if(resp == nullptr) {
			return RequestHandler::ERROR;
		}
		BOOST_LOG_TRIVIAL(trace) << "response received";
		if(resp->GetStatusCode() == Response::ResponseCode::found) {
			std::string url = resp->get_header("Location");
			if(url == "") { //error, response syntax doesn't tell us where to go
				BOOST_LOG_TRIVIAL(error) << "Error in redirect received, location to go to not specified";
				return RequestHandler::ERROR;
			}
			


			std::string::size_type protocol_pos = url.find("//");
			if(protocol_pos == std::string::npos) {
				BOOST_LOG_TRIVIAL(error) << "response didn't specify protocol.'";
				return RequestHandler::ERROR;
			}

			std::string::size_type host_pos = url.find('/', protocol_pos + 2);
			if(host_pos != std::string::npos) {
				new_host = url.substr(protocol_pos + 2, host_pos - protocol_pos - 2);
				new_uri = url.substr(host_pos);
			}
			else {
				new_host = url.substr(protocol_pos + 2);
				new_uri = "/";
			}
			
			
			OutgoingRequest.set_header(std::make_pair("Host", new_host));
			OutgoingRequest.set_uri(new_uri);
		}
		else {
			break;
		}
	}
	
	
	// This is horribly inefficient, as we make a copy. 
	if(resp.get() == nullptr) {
		return RequestHandler::ERROR;
	}
	

	*(response) = *(resp.get());
	
	return RequestHandler::OK;
}


Request ReverseProxyHandler::TransformIncomingRequest(const Request& request) const {
	Request transformed_request(request);
	transformed_request.set_header(std::make_pair("Host", host_));
	transformed_request.remove_header("Cookie"); // Passing arbitrary cookies will cause many websites to crash
	transformed_request.set_header(std::make_pair("Connection", "close")); // Passing arbitrary cookies will cause many websites to crash	
	std::string new_uri = path_;
	if(request.uri().length() > prefix_.length()) {
		if(path_.back() == request.uri().substr(prefix_.length()).front()
			&& request.uri().length() > prefix_.length() + 1) {
			new_uri += request.uri().substr(prefix_.length() + 1);
		}
		else {
			new_uri += request.uri().substr(prefix_.length());
		}
	}
	transformed_request.set_uri(new_uri); 
	return transformed_request;
}

std::unique_ptr<Response> ReverseProxyHandler::VisitOutsideServer(const Request& request, std::string host, std::string service) const {
	HTTPClient c;
	BOOST_LOG_TRIVIAL(debug) << "Binding connection to " << host << " and with service " << service;
	if(!c.EstablishConnection(host, service)) {
		return nullptr;
	}
	auto resp = c.SendRequest(request);
	return resp;
}
