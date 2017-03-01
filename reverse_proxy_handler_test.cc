#include <string>
#include <sstream>

#include "gtest/gtest.h"
#include "reverse_proxy_handler.h"
#include "request_handler.h"
#include "response.h"
#include "request.h"
#include "config_parser.h"


//Test fixture for creating configs and init RequestProxyHandler
class ReverseProxyHandlerTest : public ::testing::Test {
protected:
	bool parseString(const std::string config_string) {
		std::stringstream config_stream(config_string);
		return parser.Parse(&config_stream, &out_config);
	}

	RequestHandler::Status initProxy(std::string prefix){
		return proxy_handler.Init(prefix,out_config);
	}

	std::string getPrefix(){
		return proxy_handler.prefix_;
	}

	std::string getProtocol(){
		return proxy_handler.protocol_;
	}

	std::string getHost(){
		return proxy_handler.host_;
	}

	std::string getPath(){
		return proxy_handler.path_;
	}

	std::string getUrl(){
		return proxy_handler.url_;
	}

	Request TransformIncomingRequest(const Request& request) const{
		return proxy_handler.TransformIncomingRequest(request);
	}

	NginxConfigParser parser;
	NginxConfig out_config;
	ReverseProxyHandler proxy_handler;
};

TEST_F(ReverseProxyHandlerTest, Init) {
	parseString("proxy_pass http://www.ucla.edu/static;");
	auto init_status = initProxy("/proxy");
	EXPECT_EQ(init_status,RequestHandler::OK);
	EXPECT_EQ(getPrefix(), "/proxy");
	EXPECT_EQ(getProtocol(), "http");
	EXPECT_EQ(getHost(), "www.ucla.edu");
	EXPECT_EQ(getPath(), "/static");
	EXPECT_EQ(getUrl() , "http://www.ucla.edu/static");
}

TEST_F(ReverseProxyHandlerTest, IllegalConfig) {
	parseString("proxy_ http://www.ucla.edu;");
	auto init_status = initProxy("/proxy");
	EXPECT_EQ(init_status,RequestHandler::ERROR);

	parseString("proxy_pass;");
	init_status = initProxy("/proxy");
	EXPECT_EQ(init_status,RequestHandler::ERROR);
}

TEST_F(ReverseProxyHandlerTest, NoProtocol) {
	parseString("proxy_pass www.ucla.edu;");
	auto init_status = initProxy("/proxy");
	EXPECT_EQ(init_status,RequestHandler::ERROR);
}

//Issue 1: There are more \r\n then there should be at the end of the transformed request
TEST_F(ReverseProxyHandlerTest, TransformRequest){
	parseString("proxy_pass http://www.ucla.edu/static;");
	initProxy("/proxy");
	auto req = Request::Parse("GET /proxy/yo.txt HTTP/1.1\r\n\r\n");
	std::string new_req = TransformIncomingRequest(*req).ToString();
	EXPECT_EQ("GET /static/yo.txt HTTP/1.1\r\r\nHost: www.ucla.edu\r\nConnection: close\r\n\r\n\r\n", new_req);

}

//Issue 2: if the path is / the request uri is not correct
TEST_F(ReverseProxyHandlerTest, TransformRequestNoPath){
	parseString("proxy_pass http://www.ucla.edu;");
	initProxy("/proxy");
	auto req = Request::Parse("GET /proxy/yo.txt HTTP/1.1\r\n\r\n");
	std::string new_req = TransformIncomingRequest(*req).ToString();
	EXPECT_EQ("GET /yo.txt HTTP/1.1\r\r\nHost: www.ucla.edu\r\nConnection: close\r\n\r\n\r\n", new_req);

}

//the rest of reverse_proxy_handler should be tested in integration test
