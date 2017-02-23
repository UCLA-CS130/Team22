#include <boost/asio.hpp>
#include "gtest/gtest.h"
#include "config_parser.h"
#include "server.h"
#include "connection.h"


//Test various server configs
class MakeServerTest : public ::testing::Test {
protected:
    Server* parseConfigString(const std::string config_string) {
        std::stringstream config_stream(config_string);
        if(!parser.Parse(&config_stream, &out_config)) {
            return nullptr;
        }

        Server* server = Server::MakeServer(io_service, out_config);
        return server;
    }


    boost::asio::io_service io_service;
    NginxConfigParser parser;
    NginxConfig out_config;
};

//Test for valid config
TEST_F(MakeServerTest, ValidConfig) {
    Server* s = parseConfigString("port 8080;");
    EXPECT_TRUE(s);
    delete s;
}

// Test for invalid port
TEST_F(MakeServerTest, InvalidStringPortConfig) {
    EXPECT_FALSE(parseConfigString("port hello;"));
    EXPECT_FALSE(parseConfigString("port 68000;"));
}

// Test for missing port definition
TEST_F(MakeServerTest, MissingPortConfig) {
    EXPECT_FALSE(parseConfigString(""));
}
//test handler population is working
TEST_F(MakeServerTest, ValidHandlers) {
    Server* s = parseConfigString("port 8080; path /static StaticHandler { root static; } path /echo EchoHandler {}");
    ASSERT_TRUE(s);
    delete s;

    s = parseConfigString("port 8080; default NotFoundHandler {}");
    ASSERT_TRUE(s);
    delete s;
}

//test duplicate URIs
TEST_F(MakeServerTest, DuplicateURIs) {
    EXPECT_FALSE(parseConfigString("port 8080; path /test StaticHandler { root static; } path /test EchoHandler {}"));
    EXPECT_FALSE(parseConfigString("port 8080; default NotFoundHandler {} default AnotherNotFoundHandler {}"));
}
