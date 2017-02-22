#include <boost/asio.hpp>
#include "gtest/gtest.h"
#include "config_parser.h"
#include "server.h"
#include "connection.h"


//Test various server configs
class MakeServerTest : public ::testing::Test {
protected:
  bool parseConfigString(const std::string config_string) {
    std::stringstream config_stream(config_string);
    if(!parser.Parse(&config_stream, &out_config)) {
    	return false;
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
	EXPECT_TRUE(parseConfigString("port 8080;"));
}

// Test for invalid string as port
TEST_F(MakeServerTest, InvalidStringPortConfig) {
	EXPECT_FALSE(parseConfigString("port hello;"));
}

// Test for missing port definition
TEST_F(MakeServerTest, MissingPortConfig) {
	EXPECT_FALSE(parseConfigString(""));
}
