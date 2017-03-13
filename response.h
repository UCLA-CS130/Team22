#ifndef RESPONSE_H
#define RESPONSE_H

// Represents an HTTP response.
//
// Usage:
//   Response r;
//   r.SetStatus(RESPONSE_200);
//   r.SetBody(...);
//   return r.ToString();
//
// Constructed by the RequestHandler, after which the server should call ToString
// to serialize.

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

class Response {
public:
	enum ResponseCode {
		ok = 200,
		moved_permanently = 301,
		found = 302,
		bad_request = 400,
		unauthorized  = 401,
		not_found = 404,
		internal_server_error = 500,
		other = 600
	};
	static const std::unordered_map<ResponseCode, const std::string, std::hash<int>> status_strings;

	Response(std::string);
	Response();
	Response& operator=(const Response& rhs);

	static std::unique_ptr<Response> Parse(const std::string& raw_res);

	// sets status code of HTTP response
	void SetStatus(const ResponseCode response_code);

	// adds a header to the HTTP response
	void AddHeader(const std::string& header_name, const std::string& header_value);

	// sets the body of the HTTP response
	void SetBody(const std::string& body);

	// get the status code that was given back
	ResponseCode GetStatusCode() const;

	// convert response to a string
	std::string ToString() const;

	static ResponseCode IntToResponseCode(int code);

	std::string get_header(const std::string key);
private:
	//parse the first line of the request, involving GET,POST,etc
	bool parse_first_line(const std::string& line);

	//parse the entire raw request and update the private member variables
	bool parse_raw_response(const std::string& raw_res);
	std::vector<std::pair<std::string, std::string>> response_headers_;
	ResponseCode response_status_;
	std::string response_status_first_line_;
	std::string body_;
	std::string raw_response_;
	std::string version_;
	std::string response_status_string_;
};

#endif // RESPONSE_H
