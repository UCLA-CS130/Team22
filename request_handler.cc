#include <string>
#include <map>
#include <memory>

#include "request_handler.h"
#include "echo_handler.h"
#include "static_handler.h"
#include "not_found_handler.h"

std::map<std::string, RequestHandler* (*)(void)>* request_handler_builders = new std::map<std::string, RequestHandler* (*)(void)>();

// Creation Magic to find handler creation given the string of the class name
RequestHandler* RequestHandler::CreateByName(const std::string& type) {
	const auto type_and_builder = request_handler_builders->find(type);
	if (type_and_builder == request_handler_builders->end()) {
		return nullptr;
	}
	return (*type_and_builder->second)();
}
