#include <string>

#include "gtest/gtest.h"
#include "file_handler.h"
#include "request.h"
#include "response.h"

TEST(FileHandlerTest, SimpleTest) {
	std::string raw_req = "GET /static/kinkakuji.jpg HTTP/1.1\r\n\r\n";

	// generate header info from the request
	auto request = Request::Parse(raw_req.c_str());

	// create an echo handler
	FileHandler fileHandler("static/");

	Response response = fileHandler.HandleRequest(*request);
	std::string response_string = response.ToString();
	
	int index = response_string.find("\r\n");
	EXPECT_EQ(response_string.substr(0, index), "HTTP/1.1 200 OK");
}

TEST(FileHandlerTest, Simple404Test) {
	std::string raw_req = "GET /static/error.jpg HTTP/1.1\r\n\r\n";

	// generate header info from the request
	auto request = Request::Parse(raw_req.c_str());

	// create an echo handler
	FileHandler fileHandler("static/");

	Response response = fileHandler.HandleRequest(*request);
	std::string response_string = response.ToString();

	int index = response_string.find("\r\n");
	EXPECT_EQ(response_string.substr(0, index), "HTTP/1.1 404 Not Found");
}

TEST(FileHandlerTest, UnsupportedTest) {
	std::string raw_req = "GET /static/file_handler.o HTTP/1.1\r\n\r\n";
	auto request = Request::Parse(raw_req.c_str());

	FileHandler fileHandler("static/");

	Response response = fileHandler.HandleRequest(*request);
	std::string response_string = response.ToString();

	int index = response_string.find("\r\n");
	EXPECT_EQ(response_string.substr(0, index), "HTTP/1.1 404 Not Found");
}