#include <string>
#include <sstream>

#include "gtest/gtest.h"
#include "static_handler.h"
#include "request.h"
#include "response.h"
#include "config_parser.h"


//Test fixture for creating configs and init StaticHandler
class StaticHandlerTest : public ::testing::Test {
protected:
	bool parseString(const std::string config_string) {
		std::stringstream config_stream(config_string);
		return parser.Parse(&config_stream, &out_config);
	}

	std::string runRequest(std::string raw_req) {
		// generate header info from the request
		auto request = Request::Parse(raw_req.c_str());

		// create an echo handler
		StaticHandler staticHandler;
		staticHandler.Init("static/", out_config);

		Response response;
		staticHandler.HandleRequest(*request, &response);
		std::string response_string = response.ToString();
		
		int index = response_string.find("\r\n");
		return response_string.substr(0, index);
	}

	NginxConfigParser parser;
	NginxConfig out_config;
};

TEST_F(StaticHandlerTest, SimpleTest) {
	parseString("root static/;");
	std::string request_string = "GET /static/kinkakuji.jpg HTTP/1.1\r\n\r\n";
	std::string result = runRequest(request_string);
	
	EXPECT_EQ(result, "HTTP/1.1 200 OK");
}

TEST_F(StaticHandlerTest, Simple404Test) {
	parseString("root static/;");
	std::string request_string = "GET /static/error.jpg HTTP/1.1\r\n\r\n";
	std::string result = runRequest(request_string);

	EXPECT_EQ(result, "HTTP/1.1 404 Not Found");
}

TEST_F(StaticHandlerTest, UnsupportedTest) {
	parseString("root static/;");
	std::string request_string = "GET /static/static_handler.o HTTP/1.1\r\n\r\n";
	std::string result = runRequest(request_string);

	EXPECT_EQ(result, "HTTP/1.1 404 Not Found");
}