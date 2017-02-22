#include <string>

#include "gtest/gtest.h"
#include "not_found_handler.h"

TEST(NotFoundHandlerTest, SimpleString) {
	std::string raw_req = "";

	// generate header info from the request
	auto request = Request::Parse(raw_req.c_str());
	// create an echo handler
	NotFoundHandler not_found_handler;

	Response response;
	not_found_handler.HandleRequest(*request, &response);
	std::string response_string = response.ToString();

	std::string expected = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 15\r\n\r\n404 NOT FOUND\r\n";
	EXPECT_EQ(response_string, expected);
}
