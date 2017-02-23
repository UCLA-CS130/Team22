#ifndef STATUS_HANDLER_H
#define STATUS_HANDLER_H

#include <string>
#include <sstream>

#include "request_handler.h"
#include "config_parser.h"
#include "request.h"
#include "server.h"

class StatusHandler : public RequestHandler {
public:
	virtual RequestHandler::Status Init(const std::string& uri_prefix, const NginxConfig& config);
	virtual RequestHandler::Status HandleRequest(const Request& request, Response* response) const;

	// This handler has special initialization because it uses shared data
	void InitStatusHandler(Server*);

	static std::string StatusToHtml(const Server::Status& status);

private:
	template <typename A, typename B>
	static void HttpPrintMap(std::stringstream& out, const std::map<A,B>& map);

	Server* server_;
};

REGISTER_REQUEST_HANDLER(StatusHandler);

template<typename A, typename B>
inline void StatusHandler::HttpPrintMap(std::stringstream& out, const std::map<A, B>& map)
{
	out << "<ul style = \"list-style-type:none\">\n";
	for (auto& mapPair : map) {
		out << "<li>" << mapPair.first << " : " << mapPair.second << "</li>\n";
	}
	out << "</ul>\n";
}

#endif // STATUS_HANDLER_H
