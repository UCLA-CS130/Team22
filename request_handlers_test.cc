#include <string>

#include "gtest/gtest.h"
#include "request_handlers.h"
#include "request.h"
#include "response.h"


//----------------------------------------------------
//	EchoHandler
//----------------------------------------------------

TEST(EchoHandlerTest, SimpleString) {
	std::string raw_req = "GET /echo HTTP/1.1\r\n\r\n";

	// generate header info from the request
	auto request = Request::Parse(raw_req.c_str());

	// create an echo handler
	EchoHandler echoHandler;

	Response response;
	echoHandler.HandleRequest(*request, &response);
	std::string response_string = response.ToString();

	std::string expected = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 22\r\n\r\nGET /echo HTTP/1.1\r\n\r\n";
	EXPECT_EQ(response_string, expected);
}


//----------------------------------------------------
//	FileHandler
//----------------------------------------------------

TEST(FileHandlerTest, SimpleTest) {
	std::string raw_req = "GET /static/kinkakuji.jpg HTTP/1.1\r\n\r\n";

	// generate header info from the request
	auto request = Request::Parse(raw_req.c_str());

	// create an echo handler
	FileHandler fileHandler("static/");

	Response response;
	fileHandler.HandleRequest(*request, &response);
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

	Response response;
	fileHandler.HandleRequest(*request, &response);
	std::string response_string = response.ToString();

	int index = response_string.find("\r\n");
	EXPECT_EQ(response_string.substr(0, index), "HTTP/1.1 404 Not Found");
}

TEST(FileHandlerTest, UnsupportedTest) {
	std::string raw_req = "GET /static/file_handler.o HTTP/1.1\r\n\r\n";
	auto request = Request::Parse(raw_req.c_str());

	FileHandler fileHandler("static/");

	Response response;
	fileHandler.HandleRequest(*request, &response);
	std::string response_string = response.ToString();

	int index = response_string.find("\r\n");
	EXPECT_EQ(response_string.substr(0, index), "HTTP/1.1 404 Not Found");
}


//----------------------------------------------------
//	NotFoundHandler
//----------------------------------------------------

TEST(NotFoundHandlerTest, SimpleString) {
	std::string raw_req = "";

	// generate header info from the request
	auto request = Request::Parse(raw_req.c_str());
	// create an echo handler
	NotFoundHandler not_found_handler("");

	Response response;
	not_found_handler.HandleRequest(*request, &response);
	std::string response_string = response.ToString();

	std::string expected = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 15\r\n\r\n404 NOT FOUND\r\n";
	EXPECT_EQ(response_string, expected);
}
