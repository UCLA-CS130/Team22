#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <vector>
#include <memory>

typedef std::vector<std::pair<std::string, std::string>> Headers;

class Request
{
public:
	Request(std::string);
	static std::unique_ptr<Request> Parse(const std::string& raw_req);

	// getters

	// returns the request as a string
	std::string raw_request() const;

	// returns the method of the request (e.g. GET, POST)
	std::string method() const;

	// returns the uri of the request
	std::string uri() const;

	// returns the version of the request
	std::string version() const;

	// returns all of the headers of the request in a vector of paired strings
	Headers headers() const;

	// returns the body of the request
	std::string body() const;

	// setters

	// sets the header with the key. If it doesn't exist, the header is added
	void set_header(std::pair<std::string, std::string> header);

	// sets the uri to the string passed in 
	void set_uri(std::string uri);

	// Converts the response to a http string
	std::string ToString() const;

private:
	//parse the first line of the request, involving GET,POST,etc
	bool parse_first_line(const std::string& line);

	//parse the entire raw request and update the private member variables
	bool parse_raw_request(const std::string& raw_req);

	//member variables
	std::string raw_request_;
	std::string method_;
	std::string path_;
	std::string version_;
	Headers fields_;
	std::string body_;

	//constants
	static const std::string line_break_;
};

#endif // REQUEST_H
