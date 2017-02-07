#include <string>

#include "gtest/gtest.h"
#include "connection.h"


TEST(ConnectionTest, SimpleString) {
	boost::asio::io_service io_service;
	HandlerContainer *handlers = new HandlerContainer();
	Connection c = Connection(io_service, handlers);
	std::string response = c.write_response("hello");
	std::string expect = "hello";
	EXPECT_EQ(response, expect);
}
