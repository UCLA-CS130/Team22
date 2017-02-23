#include <boost/asio.hpp>
#include <memory>
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
    Server* s = parseConfigString("port 8080; path /static StaticHandler {} path /echo EchoHandler {}");
    ASSERT_TRUE(s);
    delete s;

    s = parseConfigString("port 8080; default NotFoundHandler {}");
    ASSERT_TRUE(s);
    delete s;
}

//test duplicate URIs
TEST_F(MakeServerTest, DuplicateURIs) {
    EXPECT_FALSE(parseConfigString("port 8080; path /test StaticHandler {} path /test EchoHandler {}"));
    EXPECT_FALSE(parseConfigString("port 8080; default NotFoundHandler {} default AnotherNotFoundHandler {}"));
}

TEST(ServerStatusTest, SimpleStatusTest) {

	ServerStatus serverStatus;

	serverStatus.LogRequest("hello", 100);
	serverStatus.LogRequest("world", 100);
	serverStatus.LogRequest("world", 200);

	ServerStatus::Snapshot snapshot = serverStatus.GetSnapshot();
	ASSERT_EQ(snapshot.totalRequests_, 3);

	// check that the response code map is (100,2),(200,1)
	auto& codeMap = snapshot.responseCountByCode_;
	ASSERT_EQ(codeMap.size(), 2);
	auto it = codeMap.begin();
	EXPECT_EQ(it->first, 100);
	EXPECT_EQ(it->second, 2);
	it++;
	EXPECT_EQ(it->first, 200);
	EXPECT_EQ(it->second, 1);

	//// check that the url map is (hello,1),(world,2)
	auto& urlMap = snapshot.requestCountByURL_;
	ASSERT_EQ(urlMap.size(), 2);
	auto it2 = urlMap.begin();
	EXPECT_EQ(it2->first, std::string("hello"));
	EXPECT_EQ(it2->second, 1);
	it2++;
	EXPECT_EQ(it2->first, std::string("world"));
	EXPECT_EQ(it2->second, 2);

	//EXPECT_EQ(status.port, 4000);
}
