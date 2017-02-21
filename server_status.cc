#include "server_status.h"

void ServerStatus::LogRequest(int responseCode)
{
	totalResponses_++;

	// if doesn't exist insert a 1
	// pair<iterator,bool> insertPair
	auto insertPair = responseCountByStatus_.insert(std::make_pair(responseCode, 1));

	// if already exists, increment
	if (insertPair.second == false) {
		// insertPair first is a pair<code,count> iterator
		std::map<int,int>::iterator it = insertPair.first;
		it->second++;
	}
}

int ServerStatus::TotalReponses()
{
	return totalResponses_;
}

std::list<std::pair<int, int>> ServerStatus::ResponseCountByStatus()
{
	std::list<std::pair<int, int>> responseList(responseCountByStatus_.begin(), responseCountByStatus_.end());
	return responseList;
}
