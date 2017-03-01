#include "gtest/gtest.h"
#include "response.h"


TEST(ResponseTest, SimpleResponse) {
	auto response = Response::Parse("HTTP/1.1 200 OK\r\nConnection: Closed\r\n\r\n");
	EXPECT_EQ(response->GetStatusCode(), Response::ResponseCode::ok);
    EXPECT_EQ(response->ToString(), "HTTP/1.1 200 OK \r\nConnection: Closed\r\r\n\r\n");
    response->SetStatus(Response::ResponseCode::not_found);
    EXPECT_EQ(response->ToString(), "HTTP/1.1 404 Not Found\r\nConnection: Closed\r\r\n\r\n");
    response->SetBody("cheese");
    response->AddHeader("test","done");
    EXPECT_EQ(response->ToString(), "HTTP/1.1 404 Not Found\r\nConnection: Closed\r\r\ntest: done\r\n\r\ncheese");
}

TEST(ResponseTest, IntToResponseCode) {
    EXPECT_EQ(Response::IntToResponseCode(200), Response::ResponseCode::ok);
    EXPECT_EQ(Response::IntToResponseCode(302), Response::ResponseCode::found);
    EXPECT_EQ(Response::IntToResponseCode(404), Response::ResponseCode::not_found);
    EXPECT_EQ(Response::IntToResponseCode(500), Response::ResponseCode::internal_server_error);
    EXPECT_EQ(Response::IntToResponseCode(100), Response::ResponseCode::other);
}


