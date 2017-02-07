#include <string>

#include "gtest/gtest.h"
#include "file_handler.h"
#include "http_parser.h"

TEST(FileHandlerTest, SimpleTest) {
	std::string request = "GET /static/kinkakuji.jpg HTTP/1.1\r\n\r\n";

	// generate header info from the request
	auto headerInfo = HttpParser::MakeHttpParser(request.c_str());

	// create an echo handler
	FileHandler fileHandler("static/");

	std::string response = fileHandler.GenerateResponse(*headerInfo, request);
	EXPECT_TRUE(response.length() > 200);
}

TEST(FileHandlerTest, Simple404Test) {
	std::string request = "GET /static/error.jpg HTTP/1.1\r\n\r\n";

	// generate header info from the request
	auto headerInfo = HttpParser::MakeHttpParser(request.c_str());

	// create an echo handler
	FileHandler fileHandler("static/");

	std::string response = fileHandler.GenerateResponse(*headerInfo, request);
	EXPECT_FALSE(response.length() > 200);
}