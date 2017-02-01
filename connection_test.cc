#include <string>

#include "gtest/gtest.h"
#include "connection.h"

TEST(ConnectionTest, SimpleString) {
    boost::asio::io_service io_service;
    Connection c = Connection(io_service);
    char data[] = "hello";
    size_t length = 5;
    std::string response = c.handle_data_write(length, data);
    std::string expect = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 5\n\nhello";
    EXPECT_EQ(response, expect);
}
