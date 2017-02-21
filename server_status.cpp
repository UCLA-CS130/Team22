#include "server_status.h"

void ServerStatus::LogRequest(int responseCode)
{
	totalResponses_++;

	// if doesn't exist insert a 0
	// pair<iterator,bool> insertPair
	auto insertPair = responseCountByStatus_.insert(std::make_pair(responseCode, 0));

	// if already exists, increment
	if (insertPair.second == false) {
		*insertPair.first++;
	}
}

int ServerStatus::TotalReponses()
{
	return totalResponses_;
}

std::list<std::pair<int, int>> ServerStatus::ResponseCountByStatus()
{
	std::list<std::pair<int, int>> responseList(responseCountByStatus_.begin(), responseCountByStatus_.end());
	return std::list<std::pair<int, int>>();
}
