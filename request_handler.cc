#include <string>
#include <map>

#include "request_handler.h"

std::map<std::string, RequestHandler* (*)(void)>* request_handler_builders = new std::map<std::string, RequestHandler* (*)(void)>();

RequestHandler* RequestHandler::CreateByName(const std::string& type) {
	std::cout << type << std::endl;
	const auto type_and_builder = request_handler_builders->find(type);
	std::cout << "TEST10" << std::endl;
	if (type_and_builder == request_handler_builders->end()) {
		std::cout << "TEST11" << std::endl;
		return nullptr;
	}
	return (*type_and_builder->second)();
}