// An nginx config file parser.

#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <map>
#include <sstream>

class NginxConfig;

// The parsed representation of a single config statement.
class NginxConfigStatement {
public:
	std::string ToString(int depth);
	std::vector<std::string> tokens_;
	std::shared_ptr<NginxConfig> child_block_;
};

// The parsed representation of the entire config.
class NginxConfig {
public:
	std::string ToString(int depth = 0);
	std::vector<std::shared_ptr<NginxConfigStatement>> statements_;

	// simple generic find method
	// returns true on success and changes *out, returns false of failure and doesn't touch *out
	template <typename T> 
	bool Find(const std::string& key, T* out) const {
		for (auto statement : statements_)
		{
			if (statement->tokens_.size() == 2 && statement->tokens_[0] == key)
			{
				std::istringstream stream(statement->tokens_[1]);
				T tmp;
				if (!(stream >> tmp)) return false; // cast error, return false
				*out = tmp;
				return true;
			}
		}
		return false;
	}
};

// The driver that parses a config file and generates an NginxConfig.
class NginxConfigParser {
public:
	NginxConfigParser() {}

	// Take a opened config file or file name (respectively) and store the
	// parsed config in the provided NginxConfig out-param. Returns true
	// iff the input config file is valid.
	bool Parse(std::istream* config_file, NginxConfig* config);
	bool Parse(const char* file_name, NginxConfig* config);

private:
	enum TokenType {
		TOKEN_TYPE_START = 0,
		TOKEN_TYPE_NORMAL = 1,
		TOKEN_TYPE_START_BLOCK = 2,
		TOKEN_TYPE_END_BLOCK = 3,
		TOKEN_TYPE_COMMENT = 4,
		TOKEN_TYPE_STATEMENT_END = 5,
		TOKEN_TYPE_EOF = 6,
		TOKEN_TYPE_ERROR = 7
	};
	const char* TokenTypeAsString(TokenType type);

	enum TokenParserState {
		TOKEN_STATE_INITIAL_WHITESPACE = 0,
		TOKEN_STATE_SINGLE_QUOTE = 1,
		TOKEN_STATE_DOUBLE_QUOTE = 2,
		TOKEN_STATE_TOKEN_TYPE_COMMENT = 3,
		TOKEN_STATE_TOKEN_TYPE_NORMAL = 4
	};

	TokenType ParseToken(std::istream* input, std::string* value);
};

#endif // CONFIG_PARSER_H
