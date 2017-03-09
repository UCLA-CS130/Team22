#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "response.h"
#include "request.h"

#include <string>
#include <memory>
#include <boost/asio.hpp>

class HTTPClient {
public:
	bool EstablishConnection(const std::string& host, const std::string& port="80");
	std::unique_ptr<Response> SendRequest(const Request& req);
private:
	boost::asio::ip::tcp::socket* socket_;
};

#endif
