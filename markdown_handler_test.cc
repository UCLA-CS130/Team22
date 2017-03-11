#include <string>
#include <sstream>

#include "gtest/gtest.h"
#include "request_handler.h"
#include "markdown_handler.h"
#include "request.h"
#include "response.h"
#include "config_parser.h"


//Test fixture for creating configs and init MarkdownHandler
class MarkdownHandlerTest : public ::testing::Test {
protected:
	bool parseString(const std::string config_string) {
		std::stringstream config_stream(config_string);
		return parser.Parse(&config_stream, &out_config);
	}

	std::string runRequest(std::string raw_req) {
		// generate header info from the request
		auto request = Request::Parse(raw_req.c_str());

		// create an echo handler
		MarkdownHandler markdownHandler;
		if(markdownHandler.Init("/readme", out_config) == RequestHandler::OK)
		{
			Response response;
			markdownHandler.HandleRequest(*request, &response);
			std::string response_string = response.ToString();
			
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
};

TEST_F(MarkdownHandlerTest, SimpleTest) {
	parseString("file README.md;");
	std::string request_string = "GET /readme HTTP/1.1\r\n\r\n";
	std::string result = runRequest(request_string);
	
	EXPECT_EQ(result, "HTTP/1.1 200 OK");
}

TEST_F(MarkdownHandlerTest, Simple404Test) {
	parseString("file bad.md;");
	std::string request_string = "GET /readme HTTP/1.1\r\n\r\n";
	std::string result = runRequest(request_string);

	EXPECT_EQ(result, "HTTP/1.1 404 Not Found");
}
