#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>
#include <unordered_map>
#include <time.h>
#include "request_handler.h"
#include "config_parser.h"
#include "request.h"
#include "response.h"

class StaticHandler : public RequestHandler {
public:
	RequestHandler::Status Init(const std::string& uri_prefix, const NginxConfig& config);
	virtual RequestHandler::Status HandleRequest(const Request& request, Response* response);

protected:
	virtual bool ProcessFile(const std::string& path, const std::string& data, std::string *new_data, std::string *content_type) const;

private:
	// max buffer length for reading in file stream
	enum { max_length = 8192 };

	// directory path local to server
	std::string directory_;

	// prefix of uri
	std::string prefix_;

	// "database" of usernames and passwords
	std::unordered_map<std::string, std::string> authentication_map_;

	// timeout for a user. only applicable if database of names and passwords is initiated, otherwise does nothing
	time_t timeout_;

	//map cookies to when they are expired
	std::unordered_map<std::string, time_t> cookie_expiration_map_;

	//read file for list of usernames and passwords, update authentication_map_
	bool init_authentication_database(std::string file_name);

	//remove old cookies from cookie_expiration_map_
	void purge_expired_cookies();

	// create login.html page
	std::string LoginToHtml(std::string full_path, std::string reason) const;
};

REGISTER_REQUEST_HANDLER(StaticHandler);

#endif // FILE_HANDLER_H
