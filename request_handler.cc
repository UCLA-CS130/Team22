#include <string>
#include <map>
#include <memory>

#include "request_handler.h"
#include "echo_handler.h"
#include "file_handler.h"
#include "not_found_handler.h"

std::map<std::string, RequestHandler* (*)(void)>* request_handler_builders = new std::map<std::string, RequestHandler* (*)(void)>();

RequestHandler* RequestHandler::CreateByName(const std::string& type) {
	if(type == "EchoHandler")
	{
		return new EchoHandler();
	}
	else if(type == "StaticHandler")
	{
		return new FileHandler();
	}
	else if(type == "NotFoundHandler")
	{
		return new NotFoundHandler();
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
