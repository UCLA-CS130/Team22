#ifndef HttpParser_H
#define HttpParser_H

#include <map>
#include <string>

class HttpParser
{
public:
    //factory
    static HttpParser* MakeHttpParser(const char* const raw_req);

    //getters
    std::string get_path();
    std::string get_method();
    std::map<std::string, std::string> get_fields();
    std::string get_body();

private:
    HttpParser();

    //parse the entire raw request and update the private member variables
    bool parse_raw_request(const char* const raw_req);

    //parse the first line of the request, involving GET,POST,etc
    bool parse_first_line(std::string line);

    //member variables
    std::string method_;
    std::string path_;
    std::map<std::string, std::string> fields_;
    std::string body_;
};

#endif // HttpParser_H
