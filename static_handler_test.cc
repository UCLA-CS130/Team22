#include <string>
#include <sstream>

#include "gtest/gtest.h"
#include "request_handler.h"
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

	std::string runRequest(std::string uri, std::string raw_req) {
		// generate header info from the request
		auto request = Request::Parse(raw_req.c_str());

		// create an static handler
		StaticHandler staticHandler;
		if(staticHandler.Init(uri, out_config) == RequestHandler::OK)
		{
			Response response;
			staticHandler.HandleRequest(*request, &response);
			std::string response_string = response.ToString();

			std::string::size_type cookie_pos = response_string.find("Set-Cookie");
			if(cookie_pos == std::string::npos)
				cookie_found = false;
			else
				cookie_found = true;

			int index = response_string.find("\r\n");
			return response_string.substr(0, index);
		}
		else
		{
			return "";
		}
	}

	NginxConfigParser parser;
	NginxConfig out_config;
	bool cookie_found;
};

TEST_F(StaticHandlerTest, SimpleTest) {
	parseString("root static/;");
	std::string request_string = "GET /static/kinkakuji.jpg HTTP/1.1\r\n\r\n";
	std::string result = runRequest("/static", request_string);

	EXPECT_EQ(result, "HTTP/1.1 200 OK");
}

TEST_F(StaticHandlerTest, MarkdownTest) {
	parseString("root static/;");
	std::string request_string = "GET /static/README.md HTTP/1.1\r\n\r\n";
	std::string result = runRequest("/static", request_string);

	EXPECT_EQ(result, "HTTP/1.1 200 OK");
}

TEST_F(StaticHandlerTest, Simple404Test) {
	parseString("root static/;");
	std::string request_string = "GET /static/error.jpg HTTP/1.1\r\n\r\n";
	std::string result = runRequest("/static", request_string);

	EXPECT_EQ(result, "HTTP/1.1 404 Not Found");
}

TEST_F(StaticHandlerTest, UnknownFileTest) {
	parseString("root static/;");
	std::string request_string = "GET /static/bad HTTP/1.1\r\n\r\n";
	std::string result = runRequest("/static", request_string);

	EXPECT_EQ(result, "HTTP/1.1 404 Not Found");
}

TEST_F(StaticHandlerTest, UnsupportedTest) {
	parseString("root static/;");
	std::string request_string = "GET /static/static_handler.o HTTP/1.1\r\n\r\n";
	std::string result = runRequest("/static", request_string);

	EXPECT_EQ(result, "HTTP/1.1 404 Not Found");
}

TEST_F(StaticHandlerTest, BadInit) {
	parseString("");
	std::string request_string = "GET /static/kinkakuji.jpg HTTP/1.1\r\n\r\n";
	std::string result = runRequest("/static", request_string);

	EXPECT_EQ(result, "");
}

//expect non-authenticated request for a private image to return 401
TEST_F(StaticHandlerTest, UnauthenticatedGet) {
	parseString("root private;\nauthentication_list authentication.txt;\ntimeout 1;");
	std::string request_string = "GET /private/axolotl.jpg HTTP/1.1\r\n\r\n";
	std::string result = runRequest("/private", request_string);
	EXPECT_EQ(result, "HTTP/1.1 401 Unauthorized");
}

//send authentication credentials to get access
TEST_F(StaticHandlerTest, AuthenticationAccepted) {
	parseString("root private;\nauthentication_list authentication.txt;\ntimeout 1;");
	std::string request_string = "POST /private/axolotl.jpg HTTP/1.1\r\n\r\nusername=user1&password=password1";
	std::string result = runRequest("/private", request_string);
	EXPECT_EQ(result, "HTTP/1.1 302 Found");

	EXPECT_TRUE(cookie_found);
}

//send bad authentication credentials that are unauthorized
TEST_F(StaticHandlerTest, AuthenticationRejection) {
	parseString("root private;\nauthentication_list authentication.txt;\ntimeout 1;");
	std::string request_string = "POST /private/axolotl.jpg HTTP/1.1\r\n\r\nusername=hello&password=world";
	std::string result = runRequest("/private", request_string);
	EXPECT_EQ(result, "HTTP/1.1 302 Found");
	EXPECT_FALSE(cookie_found);

	request_string = "POST /private/axolotl.jpg HTTP/1.1\r\n\r\nusername=user1&password=badpassword";
	result = runRequest("/private", request_string);
	EXPECT_EQ(result, "HTTP/1.1 302 Found");
	EXPECT_FALSE(cookie_found);
}
