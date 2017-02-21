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

	//getters
	std::string raw_request() const; // TODO: Implement!
	std::string method() const;
	std::string uri() const;
	std::string version() const;
	Headers headers() const;
	std::string body() const;

private:
	//parse the entire raw request and update the private member variables
	bool parse_raw_request(const std::string& raw_req);

	//parse the first line of the request, involving GET,POST,etc
	bool parse_first_line(const std::string& line);

	//member variables
	std::string raw_request_;
	std::string method_;
	std::string path_;
	std::string version_;
	Headers fields_;
	std::string body_;
};

#endif // REQUEST_H
