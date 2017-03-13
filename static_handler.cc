#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>

#include "markdown.h"
#include "request_handler.h"
#include "request.h"
#include "static_handler.h"
#include "response.h"
#include "not_found_handler.h"
#include "config_parser.h"

#include <boost/log/trivial.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

std::unordered_map<std::string,std::string> content_mappings
{
	{ "gif", "image/gif" },
	{ "htm", "text/html" },
	{ "html", "text/html" },
	{ "jpg", "image/jpeg" },
	{ "jpeg", "image/jpeg" },
	{ "png", "image/png" },
	{ "md", "text/html" }
};


RequestHandler::Status StaticHandler::Init(const std::string& uri_prefix, const NginxConfig& config)
{
	// initializing variables
	prefix_ = uri_prefix;
	timeout_ = -1;

	for (auto statement : config.statements_)
	{
		//look for root
		if(statement->tokens_.size() == 2 && statement->tokens_[0] == "root")
		{
			directory_ = statement->tokens_[1];
		}

		//look for authetication list for private files (optional)
		else if(statement->tokens_.size() == 2 && statement->tokens_[0] == "authentication_list")
		{
			if(!init_authentication_database(statement->tokens_[1]))
			{
				BOOST_LOG_TRIVIAL(error) << "error parsing given authentication_list file";
				return RequestHandler::ERROR;
			}
		}

		//look for timeout value in seconds for authentication (optional)
		else if(statement->tokens_.size() == 2 && statement->tokens_[0] == "timeout")
		{
			//exception will be thrown and caught from main if value isnt an int
			timeout_ = std::stoi(statement->tokens_[1]);
		}
	}

	//root not set
	if(directory_ == "")
	{
		BOOST_LOG_TRIVIAL(error) << "root not specified in static handler for " << uri_prefix;
		return RequestHandler::ERROR;
	}

	return RequestHandler::OK;
}


RequestHandler::Status StaticHandler::HandleRequest(const Request& request, Response* response)
{
	BOOST_LOG_TRIVIAL(trace) << "Creating static file response";

	std::string full_path = request.uri();
	std::string file_path = directory_ + full_path.substr(prefix_.length()); // get file path relative to server

	// Check if cookie exists if authentication
	if(!authentication_map_.empty())
	{
		std::string current_cookie = "";

		// Gets the current cookie from browser request headers
		std::string cookie_string = request.get_header("Cookie");

		std::string::size_type prev_separation_pos = 0;
		std::string::size_type separation_pos = cookie_string.find(";");

		while(prev_separation_pos != std::string::npos)
		{
			std::string::size_type equal_pos = cookie_string.find("=", prev_separation_pos);
			if(equal_pos < separation_pos)
			{
				std::string key = cookie_string.substr(prev_separation_pos, equal_pos - prev_separation_pos);
				if(key == "login")
				{
					separation_pos = cookie_string.find(";", prev_separation_pos + 1);
					if(separation_pos == std::string::npos)
						current_cookie = cookie_string.substr(equal_pos + 1);
					else
						current_cookie = cookie_string.substr(equal_pos + 1, separation_pos - equal_pos - 1);

					break;
				}
			}
			if(separation_pos == std::string::npos)
				prev_separation_pos = std::string::npos;
			else
			{
				prev_separation_pos = separation_pos + 2;
				separation_pos = cookie_string.find(";", prev_separation_pos + 1);
			}
		}

		if(request.method() == "GET")
		{
			purge_expired_cookies();

			// show login page if no cookie given or current_cookie is not in map
			if(current_cookie == "" || cookie_expiration_map_.find(current_cookie) == cookie_expiration_map_.end())
			{
				std::string login_data = LoginToHtml(full_path, "Please login first");
				response->SetStatus(Response::unauthorized);
				response->AddHeader("WWW-Authenticate", "FormBased");
				response->AddHeader("Content-Type", "text/html");
				response->AddHeader("Content-Length", std::to_string(login_data.length()));
				response->SetBody(login_data);
				return RequestHandler::OK;
			}
			// else, login cookie exists and is valid so continue to serve normally
		}

		else if(request.method() == "POST")
		{
			// Go through Post body to find login and password
			std::string login_body = request.body();

			std::string::size_type prev_separation_pos = 0;
			std::string::size_type separation_pos = login_body.find("&");

			std::string username = "";
			std::string password = "";

			while(prev_separation_pos != std::string::npos)
			{
				std::string::size_type equal_pos = login_body.find("=", prev_separation_pos);
				if(equal_pos < separation_pos)
				{
					std::string key = login_body.substr(prev_separation_pos, equal_pos - prev_separation_pos);
					if(key == "username")
					{
						separation_pos = login_body.find("&", prev_separation_pos + 1);
						if(separation_pos == std::string::npos)
							username = login_body.substr(equal_pos + 1);
						else
							username = login_body.substr(equal_pos + 1, separation_pos - equal_pos - 1);
					}
					else if(key == "password")
					{
						separation_pos = login_body.find("&", prev_separation_pos + 1);
						if(separation_pos == std::string::npos)
							password = login_body.substr(equal_pos + 1);
						else
							password = login_body.substr(equal_pos + 1, separation_pos - equal_pos - 1);
					}
					else
					{
						// bad argument found in post body
						std::string login_data = LoginToHtml(full_path, "Bad request. Please try logging in again");
						response->SetStatus(Response::bad_request);
						response->AddHeader("Content-Type", "text/html");
						response->AddHeader("Content-Length", std::to_string(login_data.length()));
						response->SetBody(login_data);
						return RequestHandler::OK;
					}
				}
				if(separation_pos == std::string::npos)
					prev_separation_pos = std::string::npos;
				else
				{
					prev_separation_pos = separation_pos + 1;
					separation_pos = login_body.find(";", prev_separation_pos + 1);
				}
			}

			// If authenticated properly, assign cookie as part of response and serve file normally
			auto got = authentication_map_.find(username);
			if(got != authentication_map_.end() && got->second == password) {

				// assign cookie, regenernate cookie if cookie already happens to belong to another user
				std::string new_cookie;
				do
				{
					boost::uuids::random_generator gen;
					boost::uuids::uuid u = gen();
					new_cookie = boost::uuids::to_string(u);
				} while (cookie_expiration_map_.find(new_cookie) != cookie_expiration_map_.end());

				// generate expiration time
				std::time_t expire_time = std::time(nullptr) + timeout_;
				struct tm * timeinfo = std::gmtime(&expire_time);

				//insert cookie with its expiration time in map
				cookie_expiration_map_[new_cookie] = expire_time;

				char buffer[80];
				strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S %Z", timeinfo);

				response->AddHeader("Set-Cookie", "login=" + new_cookie + "; path=" + prefix_ + "; expires=" + buffer);
			}
			// Else, respond with login page because failed authentication
			else
			{
				std::string login_data = LoginToHtml(full_path, "Username and/or password invalid");
				response->SetStatus(Response::unauthorized);
				response->AddHeader("WWW-Authenticate", "FormBased");
				response->AddHeader("Content-Type", "text/html");
				response->AddHeader("Content-Length", std::to_string(login_data.length()));
				response->SetBody(login_data);
				return RequestHandler::OK;
			}
		}
	}

	// Make sure extension exists
	std::size_t last_dot_pos = file_path.find_last_of(".");
	if(last_dot_pos == std::string::npos)
	{
		BOOST_LOG_TRIVIAL(error) << "Unknown file: " << file_path;

		NotFoundHandler not_found_handler;
		return not_found_handler.HandleRequest(request, response);
	}
	else
	{
		// Check for how to serve extension
		std::string file_extension = file_path.substr(last_dot_pos + 1);

		std::unordered_map<std::string,std::string>::const_iterator it = content_mappings.find(file_extension);
		if (it == content_mappings.end())
		{
			BOOST_LOG_TRIVIAL(error) << "Extension not supported: " << file_extension;
			NotFoundHandler not_found_handler;
			return not_found_handler.HandleRequest(request, response);
		}
		else
		{
			// Set content type and find content length, then read file and place in response
			std::string content_type = it->second;

			std::ifstream infile(file_path.c_str());
			if (infile.is_open())
			{
				response->SetStatus(Response::ok);
				response->AddHeader("Content-Type", content_type);

				std::string body_data = "";

				// Convert to html if in markdown
				if(file_extension == "md")
				{
					std::ostringstream out;
					markdown::Document doc;
					doc.read(infile);
					doc.write(out);
					body_data = out.str();
				}
				else
				{
					char buf[max_length];
					while (infile.read(buf, sizeof(buf)).gcount() > 0) {
						body_data.append(buf, infile.gcount());
					}
				}

				response->AddHeader("Content-Length", std::to_string(body_data.length()));
				response->SetBody(body_data);
			}
			else
			{
				BOOST_LOG_TRIVIAL(error) << "Unable to open file: " << file_path;
				NotFoundHandler not_found_handler;
				return not_found_handler.HandleRequest(request, response);
			}
		}
	}

	return RequestHandler::OK;
}

bool StaticHandler::init_authentication_database(std::string file_name)
{
	std::string line;
	std::ifstream file(file_name);

	while(std::getline(file, line))
	{
		size_t index_of_comma = line.find(",");
		if (index_of_comma == std::string::npos)
		{
			BOOST_LOG_TRIVIAL(error) << "Comma missing in authentication file";
			return false;
		}

		std::string username = line.substr(0, index_of_comma);
		std::string password = line.substr(index_of_comma+1);

		//check if username already exists
		if(authentication_map_.find(username) != authentication_map_.end())
		{
			BOOST_LOG_TRIVIAL(error) << "Unallowed duplicate usernames exist";
			return false;
		}
		else
		{
			authentication_map_[username] = password;
		}
	}


	if(authentication_map_.size() == 0)
	{
		BOOST_LOG_TRIVIAL(error) << "No username password pairs found";
		return false;
	}
	return true;
}

void StaticHandler::purge_expired_cookies()
{
	//iterate through each element in map, remove those with expired values
	for (auto it = cookie_expiration_map_.begin(); it != cookie_expiration_map_.end();)
	{
		time_t current_time = time(NULL);
		if(difftime(it->second, current_time) <= 0.0)
			it = cookie_expiration_map_.erase(it);
		else
			it++;
	}
}

std::string StaticHandler::LoginToHtml(std::string full_path, std::string reason) const
{
	std::stringstream body;

	// html stuff
	body << "<!DOCTYPE HTML>\n";

	body << "<html>\n";
	body << "<head>\n";
	body << "<title>Login Page</title>\n";
	body << "</head>\n";
	body << "<body style='padding-left: 20px;''>\n";
	body << "<h1>Login</h1>\n";
	body << "<h4>" << reason << "</h4>\n";

	body << "<form action='" << full_path << "' method='post'>\n";
	body << "Username:<br>\n";
	body << "<input type='text' name='username' required><br>\n";
	body << "Password:<br>\n";
	body << "<input type='password' name='password' required>\n";
	body << "<br><br>\n";
	body << "<input type='submit' value='Submit'>\n";

	body << "</form>\n";
	body << "</body>\n";
	body << "</html>\n";

	return body.str(); // is there a better way?
}
