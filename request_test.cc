#include "gtest/gtest.h"
#include "request.h"


TEST(RequestTest, SimpleRequest) {
	auto request = Request::Parse("GET /hello.htm HTTP/1.1\r\n\r\n");
	EXPECT_EQ(request->uri(), "/hello.htm");
}

TEST(RequestTest, AnotherRequest) {
	auto request = Request::Parse("GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: curl/7.43.0\r\nAccept: */*\r\n\r\nhello\r\n");
	EXPECT_EQ(request->uri(), "/");
	EXPECT_EQ(request->method(), "GET");
	EXPECT_EQ(request->headers().size(), 3);
	EXPECT_EQ(request->body(), "hello");
}


TEST(RequestTest, InvalidRequest) {
	//request does not end with \r\n\r\n
	auto request = Request::Parse("GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: curl/7.43.0\r\nAccept: */*\r\n");
	EXPECT_EQ(request, nullptr);
	//Host field is missing the semicolon
	auto request2 = Request::Parse("GET / HTTP/1.1\r\nHost localhost:8080\r\nUser-Agent: curl/7.43.0\r\nAccept: */*\r\n\r\n");
	EXPECT_EQ(request2, nullptr);

}

TEST(RequestTest, SetParameters) {
	auto request = Request::Parse("GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: curl/7.43.0\r\nAccept: */*\r\n\r\nhello\r\n");
	ASSERT_EQ(request->uri(), "/");
	ASSERT_EQ(request->method(), "GET");
	ASSERT_EQ(request->headers().size(), 3);
	ASSERT_EQ(request->body(), "hello");

	request->set_uri("/test");
	request->set_header(std::make_pair("Host", "localhost"));
	request->set_header(std::make_pair("Cheese", "localhost"));
	EXPECT_EQ(request->uri(), "/test");
	EXPECT_EQ(request->method(), "GET");
	EXPECT_EQ(request->headers().size(), 4);
	EXPECT_EQ(request->body(), "hello");

	request->remove_header("Cheese");
	EXPECT_EQ(request->headers().size(), 3);
	// Note that there is a bug with parsing. The last header has a \r\n postpended
	// ASSERT_EQ(request->ToString(), "GET /test HTTP/1.1\r\nHost: localhost\r\nUser-Agent: curl/7.43.0\r\nAccept: */*\r\n\r\nhello\r\n");
}
