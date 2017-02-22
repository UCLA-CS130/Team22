#include <string>
#include <map>
#include <memory>

#include "request_handler.h"
#include "echo_handler.h"
#include "file_handler.h"
#include "not_found_handler.h"

std::map<std::string, RequestHandler* (*)(void)>* request_handler_builders = new std::map<std::string, RequestHandler* (*)(void)>();

std::shared_ptr<RequestHandler> RequestHandler::CreateByName(const std::string& type) {
	if(type == "EchoHandler")
	{
		return std::make_shared<EchoHandler>();
	}
	else if(type == "StaticHandler")
	{
		return std::make_shared<FileHandler>();
	}
	else if(type == "NotFoundHandler")
	{
		return std::make_shared<NotFoundHandler>();
	}
	else
	{
		std::cerr << "Handler not found" << std::endl;
		return nullptr;
	}

	// const auto type_and_builder = request_handler_builders->find(type);
	// if (type_and_builder == request_handler_builders->end()) {
	// 	return nullptr;
	// }
	// return (*type_and_builder->second)();
}