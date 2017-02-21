#include "gtest/gtest.h"
#include "server_status.h"

TEST(ServerStatusTest, SimpleRequest) {
	ServerStatus status;

	status.LogRequest(100);
	status.LogRequest(100);
	status.LogRequest(200);

	int total = status.TotalReponses();
	EXPECT_EQ(total, 3);

	auto list = status.ResponseCountByStatus();
	ASSERT_EQ(list.size(), 2);

	auto it = list.begin();
	EXPECT_EQ(*it, std::make_pair(100,2));

	it++;
	EXPECT_EQ(*it, std::make_pair(200,1));

}