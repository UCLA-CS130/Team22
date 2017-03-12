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



bool HandlerContainer::AddPath(const std::string& prefix, const RequestHandler* handler){
	auto insert_result = paths_.insert(std::make_pair(prefix, std::unique_ptr<const RequestHandler>(handler)));
	return insert_result.second;
}

const RequestHandler* HandlerContainer::Find(const std::string& path) const {

	// scan the regex paths for matches
	// TODO

	// scan the non-regex paths for matches

	//exact match
	auto match = paths_.find(path);
	if(match != paths_.end())
		return match->second.get();

	//longest prefix match
	std::string prefix = get_prefix(path);
	match = paths_.find(prefix);
	if(match != paths_.end())
		return match->second.get();

	// couldn't find anything
	return nullptr;
}

std::list<std::string> HandlerContainer::GetList() const{
	std::list<std::string> list;
	for (const auto& pair : paths_) {
		list.push_back(pair.first);
	}
	for (const auto& pair : regex_paths_) {
		list.push_back(pair.first);
	}
	return list;
}

// gets longest matching prefix
std::string HandlerContainer::get_prefix(const std::string& uri) const 
{
	std::string longest = "";
	size_t index = uri.find_last_of("/");
	std::string prefix = uri.substr(0, index);

	for (auto& handlerPair : paths_)
	{
		std::string uri_key = handlerPair.first;

		//uri_key is within this prefix
		if(prefix.find(uri_key) == 0 && uri_key.length() > longest.length())
			longest = uri_key;
	}

	return longest;
}