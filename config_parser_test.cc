#include "gtest/gtest.h"
#include "config_parser.h"

TEST(NginxConfigParserTest, SimpleConfig) {
	NginxConfigParser parser;
	NginxConfig out_config;

	bool success = parser.Parse("example_config", &out_config);

	EXPECT_TRUE(success);
}

//test nested directives
TEST(NginxConfigParserTest, NestedConfig) {
	NginxConfigParser parser;
	NginxConfig out_config;
	bool success = parser.Parse("config_with_nested", &out_config);

	ASSERT_TRUE(success); //this was fixed to allow nested Configs that end like }}
	EXPECT_EQ(out_config.statements_.size(), 1);
}

TEST(NginxConfigParserTest, LargeConfig) {
	NginxConfigParser parser;
	NginxConfig out_config;

	EXPECT_TRUE(parser.Parse("large_config", &out_config));
}

//Test fixture for string parsing
class NginxStringConfigTest : public ::testing::Test {
protected:
	bool parseString(const std::string config_string) {
		std::stringstream config_stream(config_string);
		return parser.Parse(&config_stream, &out_config);
	}

	bool testParseStatements(const std::string config_string) {
		parseString(config_string);
		return out_config.ParseStatements();
	}

	bool clear(){
		out_config = NginxConfig();
	}

	NginxConfigParser parser;
	NginxConfig out_config;
};

//test invalid inputs
TEST_F(NginxStringConfigTest, InvalidConfig) {
	EXPECT_FALSE(parseString("foo bar")); //no semicolon
	EXPECT_FALSE(parseString("foo {\n\tx;\n};")); //semicolon after a brace
	EXPECT_FALSE(parseString("foo \"bar; ")); //mismatched quotes
	EXPECT_FALSE(parseString("foo \'bar; ")); //mismatched single quotes
	EXPECT_FALSE(parseString("server {listen 80;")); //mismatched curly braces
	EXPECT_FALSE(parseString("server listen 80;}")); //mismatched curly braces
}

//test basic valid configs
TEST_F(NginxStringConfigTest, ValidConfigs){
	//test comments read properly
	ASSERT_TRUE(parseString("foo bar; #a comment"));
	EXPECT_EQ(out_config.statements_.size(), 1);

	out_config.statements_.clear();

	//test multiple line commands
	ASSERT_TRUE(parseString("foo\nbar\nx;"));
	EXPECT_EQ(out_config.statements_.size(), 1);
	EXPECT_EQ(out_config.statements_[0]->tokens_[0], "foo");
	EXPECT_EQ(out_config.statements_[0]->tokens_[1], "bar");
	EXPECT_EQ(out_config.statements_[0]->tokens_[2], "x");
	EXPECT_EQ(out_config.statements_[0]->tokens_.size(), 3);

}

//test that echo_path_ and file_path_ is populated properly
TEST_F(NginxStringConfigTest, ValidConfigsWithPaths) {
	std::string config =
	"server {"
		"listen 8080;\n"
		"path /echo EchoHandler;\n"
		"path /echo2 EchoHandler;\n"
		"path /static StaticFileHandler {\n"
			"root static;\n"
		"}\n"
		"path /static2 StaticFileHandler {\n"
			"root static2;\n"
		"}\n"
	"}\n";
	ASSERT_TRUE(parseString(config));
	ASSERT_TRUE(out_config.ParseStatements());

	EXPECT_EQ(out_config.GetEchoPaths()->at(0), "/echo");
	EXPECT_EQ(out_config.GetEchoPaths()->at(1), "/echo2");
	EXPECT_EQ(out_config.GetFilePaths()->at("/static"), "static");
	EXPECT_EQ(out_config.GetFilePaths()->at("/static2"), "static2");
}


TEST_F(NginxStringConfigTest, ConfigPathScan) {
	//basic with staic and echo
	ASSERT_TRUE(testParseStatements("server { listen 8080; path /echo2 EchoHandler; path /static StaticFileHandler {root static;}}"));
	clear();

	//no directory to map to /static: fail
	ASSERT_FALSE(testParseStatements("server { listen 8080; path /static StaticFileHandler {no root;}}"));
	clear();

	//allowed to have no echo or root paths
	ASSERT_TRUE(testParseStatements("server { listen 8080; }"));
}

// port test
TEST_F(NginxStringConfigTest, ConfigPortScan){
	ASSERT_TRUE(testParseStatements("server { listen 8080; }"));
	EXPECT_EQ(out_config.GetPort(), 8080);
	clear();

	ASSERT_FALSE(testParseStatements("server { listen cats; }"));
	clear();

	ASSERT_FALSE(testParseStatements("server { listen; }"));
	clear();

	ASSERT_FALSE(testParseStatements("hey;"));
	clear();

	ASSERT_FALSE(testParseStatements("server { listen 65536; }"));
	clear();

	ASSERT_FALSE(testParseStatements("server { listen -1; }"));
	clear();
}


// Tests ToString method that contains a block
TEST(NginxConfigParserTest, ToStringBlock) {
	NginxConfigStatement statement;
	statement.tokens_.push_back("server");
	statement.tokens_.push_back("{");
	statement.tokens_.push_back("listen");
	statement.tokens_.push_back("80");
	statement.tokens_.push_back(";");
	statement.tokens_.push_back("}");
	EXPECT_EQ(statement.ToString(0), "server { listen 80 ; };\n");
}
