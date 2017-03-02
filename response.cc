#include <iostream>
#include <string>
#include <sstream>
#include "response.h"


// Status lines for status codes
namespace status_string {
	const std::string ok = "HTTP/1.1 200 OK\r\n";
	const std::string bad_request = "HTTP/1.1 400 Bad Request\r\n";
	const std::string not_found = "HTTP/1.1 404 Not Found\r\n";
	const std::string internal_server_error = "HTTP/1.1 500 Internal Server Error\r\n";
}

// Sets the response status and status string accordingly
void Response::SetStatus(const ResponseCode response_code)
{
	response_status_ = response_code;
	switch(response_code)
	{
	case Response::ok:
		response_status_string_ = status_string::ok;
		break;
	case Response::bad_request:
		response_status_string_ = status_string::bad_request;
		break;
	case Response::not_found:
		response_status_string_ = status_string::not_found;
		break;
	case Response::internal_server_error:
		response_status_string_ = status_string::internal_server_error;
		break;
	}
}

// Adds a header to HTTP response
void Response::AddHeader(const std::string& header_name, const std::string& header_value)
{
	response_headers_.push_back(std::make_pair(header_name, header_value));
}

// Sets body of the response
void Response::SetBody(const std::string& body)
{
	body_ = body;
}

// Returns the response status
Response::ResponseCode Response::GetStatusCode() const
{
	return response_status_;
}

// Converts Response object to a string
std::string Response::ToString() const
{
	std::stringstream ss;
	ss << response_status_string_;
	for(auto response_header : response_headers_)
	{
		ss << response_header.first << ": " << response_header.second << "\r\n";
	}
	ss << "\r\n";
	ss << body_;

	return ss.str();
}
