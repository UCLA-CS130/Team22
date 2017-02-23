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
}
