#include <string>

#include "gtest/gtest.h"
#include "echo_handler.h"
#include "request.h"

TEST(EchoHandlerTest, SimpleString) {
	std::string raw_req = "hello";

	// generate header info from the request
	auto request = Request::Parse(raw_req.c_str());

	// create an echo handler
	EchoHandler echoHandler;

	Response response;
	echoHandler.HandleRequest(*request, &response);
	std::string response_string = response.ToString();

	std::string expected = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 5\r\n\r\nhello";
	EXPECT_EQ(response_string, expected);
}
