#include "gtest/gtest.h"
#include "http_parser.h"

TEST(http_parser, SimpleRequest) {
    HttpParser hp = HttpParser("GET /hello.htm HTTP/1.1\r\n\r\n");
    EXPECT_EQ(hp.get_path(), "/hello.htm");
}

TEST(http_parser, AnotherRequest) {
    HttpParser hp = HttpParser("GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: curl/7.43.0\r\nAccept: */*\r\n\r\nhello\r\n");
    EXPECT_EQ(hp.get_path(), "/");
    EXPECT_EQ(hp.get_method(), "GET");
    EXPECT_EQ(hp.get_fields()["User-Agent"], "curl/7.43.0");
    EXPECT_EQ(hp.get_body(), "hello");
}
