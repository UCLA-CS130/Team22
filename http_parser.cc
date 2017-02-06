#include "http_parser.h"
#include <string>
#include <vector>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

HttpParser* HttpParser::MakeHttpParser(const char* const raw_req)
{
    HttpParser* hp = new HttpParser();
    if(hp->parse_raw_request(raw_req))
    {
        return hp;
    }
    else
    {
        delete hp;
        return nullptr;
    }
}

HttpParser::HttpParser() {}

std::string HttpParser::get_path()
{
    return path_;
}

std::string HttpParser::get_method()
{
    return method_;
}

std::map<std::string, std::string> HttpParser::get_fields()
{
    return fields_;
}

std::string HttpParser::get_body()
{
    return body_;
}

bool HttpParser::parse_first_line(std::string line)
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

    return true;
}

bool HttpParser::parse_raw_request(const char* const raw_req){
    std::string req(raw_req);
    std::vector<std::string> lines;

    //separate the request body, denoted by \r\n\r\n.
    //if there is any content after, update value of body_.
    size_t end_fields_index = req.find("\r\n\r\n");

    if(end_fields_index == std::string::npos)
        return false;

    size_t begin_body_index = end_fields_index + 4; //Add 4 to skip to the body.
    if(begin_body_index < req.size())
        body_ = req.substr(begin_body_index, req.size() - begin_body_index - 2); //minus 2 to ignore the last \r\n

    //truncate req to everything before the \r\n\r\n
    req = req.substr(0, end_fields_index + 1);

    //split raw request based on /r/n
    //boost::split(lines, req, boost::is_any_of("\n"));
    boost::regex re("(\r\n)+");
    boost::sregex_token_iterator it(req.begin(), req.end(), re, -1);
    boost::sregex_token_iterator j;
    while(it != j)
        lines.push_back(*it++);

    //parse the method and path separately, return 0 if it fails
    if(!parse_first_line(lines[0]))
        return false;

    //populate the map with the fields
    for(unsigned int i = 1; i < lines.size(); ++i)
    {
        std::size_t index = lines[i].find(": ");
        if(index == std::string::npos)
            return false;

        std::string key = lines[i].substr(0, index);
        std::string value = lines[i].substr(index+2); //add 2 to skip the ": "
        fields_[key] = value;
    }

    return true;
}
