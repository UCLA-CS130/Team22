#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "response.h"
#include "request.h"

#include <string>
#include <memory>
#include <boost/asio.hpp>

class HTTPClient {
public:
	bool EstablishConnection(const std::string& host, const std::string& service="http");
	std::unique_ptr<Response> SendRequest(const Request& req);
	~HTTPClient();
private:
	boost::asio::ip::tcp::socket* socket_;
};

#endif
