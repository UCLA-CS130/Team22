#include <string>
#include <vector>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/log/trivial.hpp>
#include "request.h"

const std::string Request::line_break_ = "\r\n";

Request::Request(std::string raw_req) : raw_request_(raw_req) { }

std::unique_ptr<Request> Request::Parse(const std::string& raw_req)
{
	std::unique_ptr<Request> request(new Request(raw_req));
	if(request->parse_raw_request(raw_req))
	{
		return request;
	}
	else
	{
		return nullptr;
	}
}

// returns the request as a string
std::string Request::raw_request() const
{
	return raw_request_;
}

// returns the method of the request (e.g. GET, POST)
std::string Request::method() const
{
	return method_;
}

// returns the uri of the request
std::string Request::uri() const
{
	return path_;
}

// returns the version of the request
std::string Request::version() const
{
	return version_;
}

// returns all of the headers of the request in a vector of paired strings
Headers Request::headers() const
{
	return fields_;
}

// returns the body of the request
std::string Request::body() const
{
	return body_;
}

void Request::set_header(std::pair<std::string, std::string> header) {
	for(auto& header_ : fields_) {
		if(header_.first == header.first) {
			header_.second = header.second;
			// Exit the function as the header has been set
			return;
		}
	}
	// Header not set
	fields_.push_back(header);
}

void Request::remove_header(std::string key) {
	for(std::size_t i = 0; i < fields_.size(); i++) {
		if(fields_[i].first == key) {
			fields_.erase(fields_.begin() + i);
			return; 
		}
	}
}

void Request::set_uri(std::string uri) {
	path_ = uri;
}

std::string Request::ToString() const {
	std::string http_request;
	
	// Prepare the first line
	http_request.append(method_);
	http_request.append(" ");
	http_request.append(path_);
	http_request.append(" ");
	http_request.append(version_);
	http_request.append(line_break_);

	// Attach the headers
	for(auto const& header_ : fields_) {
		http_request.append(header_.first);
		http_request.append(": " );
		http_request.append(header_.second);
		http_request.append(line_break_);
	}

	// Beginning of Body
	http_request.append(line_break_);
	http_request.append(body_);

	return http_request;
}

//parse the first line of the request, involving GET, POST, etc
bool Request::parse_first_line(const std::string& line)
{
	std::vector<std::string> tokens;
	boost::split(tokens, line, boost::is_any_of(" "));

	unsigned int expected_num_of_tokens = 3;
	if(tokens.size() != expected_num_of_tokens)
		return false;

	if(tokens[0] != "GET" && tokens[0] != "POST")
		return false;

	method_ = tokens[0];
	path_ = tokens[1];
	version_ = tokens[2];

	return true;
}

//parse the entire raw request and update the private member variables
bool Request::parse_raw_request(const std::string& req){
	BOOST_LOG_TRIVIAL(trace) << "Parsing raw request text...";
	std::vector<std::string> lines;

	//separate the request body, denoted by \r\n\r\n.
	//if there is any content after, update value of body_.
	size_t end_fields_index = req.find("\r\n\r\n");

	if(end_fields_index == std::string::npos)
	{
		BOOST_LOG_TRIVIAL(error) << "error in raw request: couldn't find the end of the request fields preceding request body";
		return false;
	}
	size_t begin_body_index = end_fields_index + 4; //Add 4 to skip to the body.
	if(begin_body_index < req.size())
		body_ = req.substr(begin_body_index, req.size() - begin_body_index - 2); //minus 2 to ignore the last \r\n

	//truncate req to everything before the \r\n\r\n
	std::string new_req = req.substr(0, end_fields_index + 1);

	//split raw request based on /r/n
	//boost::split(lines, req, boost::is_any_of("\n"));
	boost::regex re("(\r\n)+");
	boost::sregex_token_iterator it(new_req.begin(), new_req.end(), re, -1);
	boost::sregex_token_iterator j;
	while(it != j)
		lines.push_back(*it++);


	//parse the method and path separately, return 0 if it fails
	if(!parse_first_line(lines[0]))
	{
		BOOST_LOG_TRIVIAL(error) << "error in raw request format: first line not exactly 3 tokens or uses something other than GET or POST";
		return false;
	}
	//populate the map with the fields
	for(unsigned int i = 1; i < lines.size(); ++i)
	{
		std::size_t index = lines[i].find(": ");
		if(index == std::string::npos)
		{
			BOOST_LOG_TRIVIAL(error) << "error in raw request format: field line doesn't contain a colon character";
			return false;
		}
		std::string key = lines[i].substr(0, index);
		std::string value = lines[i].substr(index+2); //add 2 to skip the ": "

		std::pair<std::string, std::string> field_pair(key, value);
		fields_.push_back(field_pair);
	}

	return true;
}
