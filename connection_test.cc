#include <string>

#include "gtest/gtest.h"
#include "connection.h"


TEST(ConnectionTest, SimpleString) {
	boost::asio::io_service io_service;
	Connection c = Connection(io_service);
	std::string response = c.write_response("hello");
	std::string expect = "hello";
	EXPECT_EQ(response, expect);
}
