#include <string>
#include <map>
#include <memory>

#include "request_handler.h"
#include "echo_handler.h"
#include "static_handler.h"
#include "not_found_handler.h"

std::map<std::string, RequestHandler* (*)(void)>* request_handler_builders = new std::map<std::string, RequestHandler* (*)(void)>();

RequestHandler* RequestHandler::CreateByName(const std::string& type) {
	const auto type_and_builder = request_handler_builders->find(type);
	if (type_and_builder == request_handler_builders->end()) {
		std::cout << "TEST1" << std::endl;
		return nullptr;
	}
	return (*type_and_builder->second)();
}
