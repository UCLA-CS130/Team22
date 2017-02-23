#include <string>
#include <sstream>
#include <memory>
#include <iostream>

#include "gtest/gtest.h"
#include "status_handler.h"
#include "request_handler.h"
#include "request.h"
#include "response.h"
#include "server.h"

//Test fixture for creating configs and init StaticHandler
class StatusHandlerFixture : public ::testing::Test {
protected:
	virtual void SetUp() {
		RequestHandler* handler = RequestHandler::CreateByName("StatusHandler");
		StatusHandler* shandler = dynamic_cast<StatusHandler*>(handler);
		statusHandler = std::unique_ptr<StatusHandler>(shandler);
	}
	RequestHandler::Status InitString(std::string uri, std::string config) {
		std::stringstream ss(config);
		parser.Parse(&ss, &out_config);
		return statusHandler->Init(uri, out_config);
	}

	NginxConfigParser parser;
	NginxConfig out_config;
	std::unique_ptr<StatusHandler> statusHandler;
};

TEST(StatusHandlerTest, SimpleTest) {
	using namespace std;

	ServerStatus::Snapshot status;
	status.port_ = 1234;
	status.requestCountByURL_ = {
		{"url1", 200},
		{"url2", 45},
	};
	status.responseCountByCode_ = {
		{200, 100140},
		{404, 6}
	};
	status.requestHandlers_ = { "one", "two", "three" };
	status.totalRequests_ = 140;

	std::string html = StatusHandler::StatusToHtml(status);
	std::cout << html;

	ASSERT_GT(html.length(), 300);
}

TEST_F(StatusHandlerFixture, StatusHandlerConstruction) {
	// can we even get this far?
	ASSERT_NE(statusHandler.get(), nullptr);

	// Init shouldn't require anything
	ASSERT_EQ(InitString("",""), RequestHandler::OK);

	// Special init
	ServerStatus status;
	statusHandler->InitStatusHandler(&status);

	// prepare the ServerStatus
	status.LogRequest("/some/url", 5555);
	status.LogRequest("/some/url", 2222);
	status.LogRequest("/some/url", 1111);
	status.LogRequest("/someother/url2", 9999);

	// Handle a request
	auto request = Request::Parse("GET /echo HTTP/1.1\r\n\r\n");
	Response response;
	statusHandler->HandleRequest(*request, &response);

	std::string result = response.ToString();

	// some general expectations for the html result
	EXPECT_GT(result.length(), 150);
	EXPECT_NE(result.find("5555"), std::string::npos);
	EXPECT_NE(result.find("9999"), std::string::npos);
	EXPECT_NE(result.find("/some/url"), std::string::npos);
	EXPECT_NE(result.find("4"), std::string::npos); // total number of requests
	EXPECT_NE(result.find("3"), std::string::npos); // count of /some/url

	//std::cout << result;
}
