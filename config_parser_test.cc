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

//test Find<>
TEST_F(NginxStringConfigTest, GenericFindTest) {
	//test comments read properly
	std::string test_string = "normal 10;\n"
		"badcast fifty;\n"
		"tooshort;\n"
		"way too long;\n"
		"afloat 14.92926;\n"
		"astring hello;\n";

	ASSERT_TRUE(parseString(test_string));

	//out_config;
	int normal = 0;
	EXPECT_EQ(out_config.Find<int>("normal", &normal), true);
	EXPECT_EQ(normal, 10);

	int badcast = 400;
	EXPECT_EQ(out_config.Find<int>("badcast", &badcast), false);
	EXPECT_EQ(badcast, 400);

	int tooshort = 12;
	EXPECT_EQ(out_config.Find<int>("tooshort", &tooshort), false);
	EXPECT_EQ(tooshort, 12);

	int way = 12;
	EXPECT_EQ(out_config.Find<int>("way", &tooshort), false);
	EXPECT_EQ(way, 12);

	float afloat;
	EXPECT_EQ(out_config.Find<float>("afloat", &afloat), true);
	EXPECT_LT(abs(afloat-14.92926), .001); // does this actually work? there might be rounding errors

	std::string astring;
	EXPECT_EQ(out_config.Find<std::string>("astring", &astring), true);
	EXPECT_EQ(astring, "hello");
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
