#include "gtest/gtest.h"
#include "request_handler.h"
#include "echo_handler.h"

TEST(RequestHandlerTest, SimpleCreate) {
	RequestHandler* handler = RequestHandler::CreateByName("EchoHandler");
	ASSERT_TRUE(handler);
}

TEST(RequestHandlerTest, BadCreate) {
	RequestHandler* handler = RequestHandler::CreateByName("BadHandler");
	ASSERT_FALSE(handler);
}
