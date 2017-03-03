#include <string>

#include "gtest/gtest.h"
#include "http_client.h"
#include "request.h"
#include "response.h"

// Note: these tests are networked in nature.
// This means if you have internet issues, 
// They may fail unexpectedly.

TEST(HttpClientTest, EstablishConnection) {
	HTTPClient c;
	EXPECT_TRUE(c.EstablishConnection("www.google.com", "http"));
	EXPECT_TRUE(c.EstablishConnection("www.google.com"));
	EXPECT_TRUE(c.EstablishConnection("www.google.com", "https"));
	EXPECT_FALSE(c.EstablishConnection("www....google.com", "http"));
	EXPECT_FALSE(c.EstablishConnection("www.google.com", "f"));
	EXPECT_FALSE(c.EstablishConnection("www.google.com", "invalid_protocol"));
	EXPECT_FALSE(c.EstablishConnection("http://www.google.com"));
}

TEST(HttpClientTest, SendRequest) {
	HTTPClient c;
	auto req= Request::Parse("GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: close\r\n\r\n");
	ASSERT_NE(req.get(), nullptr);
	ASSERT_TRUE(c.EstablishConnection("www.example.com", "http"));
	auto res = c.SendRequest(*req.get());
	EXPECT_NE(res.get(), nullptr); // Got a response?
}