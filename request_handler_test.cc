#include <string>
#include <iostream>
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

TEST(RequestHandlerTest, SimpleAddHandler) {

	HandlerContainer handlers;

	// use real pointers
	RequestHandler* x = RequestHandler::CreateByName("EchoHandler");
	RequestHandler* y = RequestHandler::CreateByName("EchoHandler");
	RequestHandler* z = RequestHandler::CreateByName("EchoHandler");
	RequestHandler* w = RequestHandler::CreateByName("EchoHandler");

	ASSERT_TRUE(handlers.AddPath("/prefix1", x));
	ASSERT_TRUE(handlers.AddPath("/prefix2", y));
	ASSERT_FALSE(handlers.AddPath("/prefix2", w));

	EXPECT_EQ(handlers.Find("/prefix1"), x);
	EXPECT_EQ(handlers.Find("/prefix2"), y);
	EXPECT_EQ(handlers.Find("/nothing"), nullptr);

	// empty path
	ASSERT_TRUE(handlers.AddPath("", z)); // use "" for the default
	EXPECT_EQ(handlers.Find(""), z);
	EXPECT_EQ(handlers.Find("/nothing"), z);

	// alphabetical order
	EXPECT_EQ(handlers.GetList(), std::list<std::string>({"","/prefix1","/prefix2"}));
}


TEST(RequestHandlerTest, AddHandlerRegex) {

	HandlerContainer handlers;

	// use real pointers
	RequestHandler* x = RequestHandler::CreateByName("EchoHandler");
	RequestHandler* y = RequestHandler::CreateByName("EchoHandler");
	RequestHandler* z = RequestHandler::CreateByName("EchoHandler");
	RequestHandler* w = RequestHandler::CreateByName("EchoHandler");
	RequestHandler* v = RequestHandler::CreateByName("EchoHandler");

	ASSERT_TRUE(handlers.AddPath("/prefix1", x));
	ASSERT_TRUE(handlers.AddRegexPath("/prefix1", "\\.lua", y));
	ASSERT_TRUE(handlers.AddRegexPath("/prefix1", "lua", z));
	ASSERT_TRUE(handlers.AddRegexPath("/foo", ".*", w));
	ASSERT_FALSE(handlers.AddRegexPath("/prefix2", "[.lua", v)); // regex mismatched [
	
	// TODO: this is probably going to change soon anyway
	std::list<std::string> list = handlers.GetList();
	// EXPECT_EQ(list, {
	// 	"/prefix1",
	// });

	EXPECT_EQ(handlers.Find("/prefix1"), x);
	EXPECT_EQ(handlers.Find("/prefix1/cats.lua"), y); // matches y, but not x
	EXPECT_EQ(handlers.Find("/prefix1/catslua"),z);
	EXPECT_EQ(handlers.Find("anything"), nullptr);
	EXPECT_EQ(handlers.Find("/foo/anything"), w); // .* doesn't match due to prefix

	// empty path
	ASSERT_TRUE(handlers.AddRegexPath("", ".*", v));
	EXPECT_EQ(handlers.Find("anything"), v); // matches "" prefix and .*

}