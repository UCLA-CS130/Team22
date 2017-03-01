#include <iostream>
#include <string>
#include <sstream>
#include "response.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/log/trivial.hpp>
#include <boost/lexical_cast.hpp>

// Status lines for status codes
namespace status_string {
	const std::string ok = "HTTP/1.1 200 OK\r\n";
	const std::string found = "HTTP/1.1 302 Found\r\n";
	const std::string not_found = "HTTP/1.1 404 Not Found\r\n";
	const std::string internal_server_error = "HTTP/1.1 500 Internal Server Error\r\n";
}

Response::Response(std::string raw_res) : raw_response_(raw_res) { }
Response::Response() { }
Response& Response::operator=(const Response& rhs) {
	// Check for self-assignment
	if(this == &rhs) {
		return *this;
	}

	this->response_headers_ = rhs.response_headers_;
	this->response_status_ = rhs.response_status_;
	this->response_status_first_line_ = rhs.response_status_first_line_;
	this->body_ = rhs.body_;
	this->raw_response_ = rhs.raw_response_;
	this->version_ = rhs.version_;
	this->response_status_string_ = rhs.response_status_string_;

	return *this;
}
std::unique_ptr<Response> Response::Parse(const std::string& raw_res)
{
	std::unique_ptr<Response> response(new Response(raw_res));
	if(response->parse_raw_response(raw_res))
	{
		return response;
	}
	else
	{
		return nullptr;
	}
}

// Sets the response status and status string accordingly
void Response::SetStatus(const ResponseCode response_code)
{
	response_status_ = response_code;
	switch(response_code)
	{
	case Response::ok:
		response_status_first_line_ = status_string::ok;
		break;
	case Response::found:
		response_status_first_line_ = status_string::found;
		break;
	case Response::not_found:
		response_status_first_line_ = status_string::not_found;
		break;
	case Response::internal_server_error:
		response_status_first_line_ = status_string::internal_server_error;
		break;
	default:
		response_status_first_line_ = status_string::internal_server_error;
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
	ss << response_status_first_line_;
	for(auto response_header : response_headers_)
	{
		ss << response_header.first << ": " << response_header.second << "\r\n";
	}
	ss << "\r\n";
	ss << body_;

	return ss.str();
}

Response::ResponseCode Response::IntToResponseCode(int code) {
	switch(code) {
		case 200:
			return ResponseCode::ok;
		case 302:
			return ResponseCode::found;
		case 404:
			return ResponseCode::not_found;
		case 500:
			return ResponseCode::internal_server_error;
		default:
			return ResponseCode::other;
	}
}
//parse the first line of the response
bool Response::parse_first_line(const std::string& line)
{
	std::vector<std::string> tokens;
	boost::split(tokens, line, boost::is_any_of(" "));

	unsigned int expected_num_of_tokens = 2;
	if(tokens.size() <= expected_num_of_tokens)
		return false;

	version_ = tokens[0]; 

	int response_code;
	try {
		response_code = boost::lexical_cast<int>(tokens[1]);
	}
	catch (const boost::bad_lexical_cast&) {
		return false;
	}
	response_status_ = IntToResponseCode(response_code);
	for(unsigned int i = 2; i < tokens.size(); i++) {
		response_status_string_ += tokens[i] + " " ;
	}

	response_status_first_line_ = tokens[0] + " " + tokens[1] + " " + response_status_string_ + "\r\n";	

	return true;
}

//parse the entire raw response and update the private member variables
bool Response::parse_raw_response(const std::string& res){
	BOOST_LOG_TRIVIAL(trace) << "Parsing raw response text...";
	std::vector<std::string> lines;

	//separate the resuest body, denoted by \r\n\r\n.
	//if there is any content after, update value of body_.
	size_t end_fields_index = res.find("\r\n\r\n");

	if(end_fields_index == std::string::npos)
	{
		BOOST_LOG_TRIVIAL(error) << "error in raw response: couldn't find the end of the response fields preceding response body";
		return false;
	}
	size_t begin_body_index = end_fields_index + 4; //Add 4 to skip to the body.
	if(begin_body_index < res.size())
		body_ = res.substr(begin_body_index, res.size() - begin_body_index);

	//truncate res to everything before the \r\n\r\n
	std::string new_res = res.substr(0, end_fields_index + 1);

	//split raw resuest based on /r/n
	//boost::split(lines, res, boost::is_any_of("\n"));
	boost::regex re("(\r\n)+");
	boost::sregex_token_iterator it(new_res.begin(), new_res.end(), re, -1);
	boost::sregex_token_iterator j;
	while(it != j)
		lines.push_back(*it++);


	//parse the method and path separately, return 0 if it fails
	if(!parse_first_line(lines[0]))
	{
		BOOST_LOG_TRIVIAL(error) << "error in raw response format: first line not exactly 3 tokens or response code not an integer";
		return false;
	}
	//populate the map with the fields
	for(unsigned int i = 1; i < lines.size(); ++i)
	{
		std::size_t index = lines[i].find(": ");
		if(index == std::string::npos)
		{
			BOOST_LOG_TRIVIAL(error) << "error in raw response format: field line doesn't contain a colon character";
			return false;
		}
		std::string key = lines[i].substr(0, index);
		std::string value = lines[i].substr(index+2); //add 2 to skip the ": "

		std::pair<std::string, std::string> field_pair(key, value);
		response_headers_.push_back(field_pair);
	}

	return true;
}
