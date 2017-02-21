#ifndef SERVER_STATUS_H
#define SERVER_STATUS_H

#include <map>
#include <list>

class ServerStatus {
public:
	// increments a counter for a particular responseCode
	void LogRequest(int responseCode);

	int TotalReponses();
	
	// returns a list of pairs (responseCode, count)
	std::list<std::pair<int, int>> ResponseCountByStatus();

private:
	int totalResponses_ = 0;
	std::map<int, int> responseCountByStatus_;

};

// TODO: make this class should be thread safe since this will be shared
// NOTE: this interface + threads = it is possible that total =/= sum of countByStatus, do we care?
//    can fix this with a mutexed GetStatus returning a snapshot struct of everything

#endif // SERVER_STATUS_H
