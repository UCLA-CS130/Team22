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

class Response {
public:
	enum ResponseCode {
		ok = 200,
        not_found = 404,
        internal_server_error = 500
	};

	void SetStatus(const ResponseCode response_code);
	void AddHeader(const std::string& header_name, const std::string& header_value);
	void SetBody(const std::string& body);

	ResponseCode GetStatusCode() const;

	std::string ToString() const;

private:
	std::vector<std::pair<std::string, std::string>> response_headers_;
	ResponseCode response_status_;
	std::string response_status_string_;
	std::string body_;
};

#endif // RESPONSE_H
