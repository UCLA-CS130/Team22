#include <string>

#include "gtest/gtest.h"
#include "connection.h"
#include "response.h"


TEST(ConnectionTest, SimpleString) {
	boost::asio::io_service io_service;
	std::unique_ptr<HandlerContainer> handlers(new HandlerContainer);
	
	Connection c(io_service, handlers.get(), NULL);

	std::string body = "hello";

	Response response;
	response.SetStatus(Response::ok);
	response.AddHeader("Content-Type", "text/plain");
	response.AddHeader("Content-Length", std::to_string(body.length()));
	response.SetBody(body);

	std::string response_result = c.write_response(response);
	std::string expect = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 5\r\n\r\nhello";
	EXPECT_EQ(response_result, expect);
}
