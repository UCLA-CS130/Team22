#ifndef REVERSE_PROXY_HANDLER_H
#define REVERSE_PROXY_HANDLER_H

#include <string>

#include "request_handler.h"
#include "config_parser.h"
#include "request.h"
#include "response.h"
#include <boost/asio.hpp>


class ReverseProxyHandler : public RequestHandler {
public:
	virtual RequestHandler::Status Init(const std::string& uri_prefix, const NginxConfig& config);
	virtual RequestHandler::Status HandleRequest(const Request& request, Response* response) const;
	
	// 21 Redirects is the standard used by most modern web browers.
	const int MaxRedirectDepth = 21;
private:
	Response VisitOutsideServer(const Request& request) const;
	// Transforms the request parameters into the Reverse Proxy paramters
	Request TransformIncomingRequest(const Request& request) const;
	std::string prefix_;
	std::string host_;
	std::string path_;
	std::string url_;
};

REGISTER_REQUEST_HANDLER(ReverseProxyHandler);


#endif // REVERSE_PROXY_HANDLER_H
