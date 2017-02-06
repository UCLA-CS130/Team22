#include "gtest/gtest.h"
#include "http_parser.h"
/*
class HttpParserTest : public ::testing:Test {
protected:
    HttpParser* ParseRequest(const std::string req)
    {
        return HttpParser::MakeHttpParser(req);
    }
}
*/

TEST(http_parser, SimpleRequest) {
    HttpParser* hp = HttpParser::MakeHttpParser("GET /hello.htm HTTP/1.1\r\n\r\n");
    EXPECT_EQ(hp->get_path(), "/hello.htm");

    delete hp;
}

TEST(http_parser, AnotherRequest) {
    HttpParser* hp = HttpParser::MakeHttpParser("GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: curl/7.43.0\r\nAccept: */*\r\n\r\nhello\r\n");
    if(hp == nullptr) {
        std::cout << "REKT";
    }
    EXPECT_EQ(hp->get_path(), "/");
    EXPECT_EQ(hp->get_method(), "GET");
    EXPECT_EQ(hp->get_fields()["User-Agent"], "curl/7.43.0");
    EXPECT_EQ(hp->get_body(), "hello");

    delete hp;
}


TEST(http_parser, InvalidRequest) {
    //request does not end with \r\n\r\n
    HttpParser* hp = HttpParser::MakeHttpParser("GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: curl/7.43.0\r\nAccept: */*\r\n");
    EXPECT_EQ(hp, nullptr);
    //Host field is missing the semicolon
    HttpParser* hp2 = HttpParser::MakeHttpParser("GET / HTTP/1.1\r\nHost localhost:8080\r\nUser-Agent: curl/7.43.0\r\nAccept: */*\r\n\r\n");
    EXPECT_EQ(hp2, nullptr);

}
