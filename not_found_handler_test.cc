#include <string>
#include <sstream>

#include "gtest/gtest.h"
#include "not_found_handler.h"
#include "request_handler.h"
#include "response.h"
#include "request.h"
#include "config_parser.h"

class NotFoundHandlerTest : public ::testing::Test {
protected:
	bool parseString(const std::string config_string) {
		std::stringstream config_stream(config_string);
		return parser.Parse(&config_stream, &out_config);
	}

	std::string runRequest(std::string raw_req) {
		// generate header info from the request
		auto request = Request::Parse(raw_req.c_str());

		// create an echo handler
		NotFoundHandler notFoundHandler;
		if(notFoundHandler.Init("/bad", out_config) == RequestHandler::OK)
		{
			Response response;
			notFoundHandler.HandleRequest(*request, &response);
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

TEST_F(NotFoundHandlerTest, SimpleRequest) {
	parseString("");
	std::string request_string = "GET /bad HTTP/1.1\r\n\r\n";
	std::string result = runRequest(request_string);

	EXPECT_EQ(result, "HTTP/1.1 404 Not Found");
}