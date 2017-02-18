#include <string>

#include "gtest/gtest.h"
#include "not_found_handler.h"

TEST(NotFoundHandlerTest, SimpleString) {
	std::string request = "";

	// generate header info from the request
	auto headerInfo = HttpParser::MakeHttpParser(request.c_str());
	// create an echo handler
	NotFoundHandler not_found_handler;

	std::string response = not_found_handler.GenerateResponse(*headerInfo, request);
	std::string expected = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 15\r\n\r\n404 NOT FOUND\r\n";
	EXPECT_EQ(response, expected);
}
