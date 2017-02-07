#include <string>

#include "gtest/gtest.h"
#include "echo_handler.h"
#include "http_parser.h"

TEST(EchoHandlerTest, SimpleString) {
	std::string request =
		"hello";

	// generate header info from the request
	auto headerInfo = HttpParser::MakeHttpParser(request.c_str());

	// create an echo handler
	EchoHandler echoHandler;

	std::string response = echoHandler.GenerateResponse(*headerInfo, request);
	std::string expected = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 5\r\n\r\nhello";
	EXPECT_EQ(response, expected);
}
