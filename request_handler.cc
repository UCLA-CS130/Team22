#include <string>
#include <map>
#include <memory>
#include <regex>

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

bool HandlerContainer::AddRegexPath(const std::string& prefix, const std::string& regex, const RequestHandler* handler){
	//auto insert_result = paths_.insert(std::make_pair(prefix, std::unique_ptr<const RequestHandler>(handler)));
	regex_paths_.push_back(std::make_tuple(prefix, regex, std::unique_ptr<const RequestHandler>(handler)));
	return true; // TODO: duplicate checking?
}

const RequestHandler* HandlerContainer::Find(const std::string& path) const {

	// scan the regex paths for matches
	// longest prefix match the list
	size_t longest = 0;
	std::string longest_prefix = "";
	for (auto& tuple : regex_paths_){
		std::string s = std::get<0>(tuple);
		if (path.find(s) == 0 && s.size() > longest) {
			longest = s.size();
			longest_prefix = s;
		}
	}
	printf("longest regex match: %zu, %s\n", longest, longest_prefix.c_str());
	// scan through again, this time applying the regex condition
	if (longest_prefix != ""){
		for (auto& tuple : regex_paths_) {
			if (path.find(longest_prefix) == 0) {
				printf("testing regex match: %s\n", );
				// TODO: save the regex parse to save time later
				if (std::regex_match(path, std::regex(std::get<1>(tuple)))){
					return std::get<2>(tuple).get();
				}
			}
		}
	}

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
	for (const auto& tuple : regex_paths_) {
		list.push_back(std::get<0>(tuple) + " " + std::get<1>(tuple));
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

