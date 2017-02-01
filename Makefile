CXX=g++
CXXFLAGS=-std=c++11 -I. -Wall -Werror -fprofile-arcs -ftest-coverage
BOOSTFLAG = -lboost_system
DEPS=server.h connection.h config_parser.h
OBJ=server.o connection.o config_parser.o main.o
TESTOBJ=server.gcno connection.gcno config_parser.gcno main.gcno

%.o: %.c $(DEPS)
	$(CC) $(CXXFLAGS) -c -o $@ $<

webserver: $(OBJ)
	g++ $(CXXFLAGS) -o $@ $^ $(BOOSTFLAG)

test:
	./build_tests.sh
	./config_parser_test

integration-test:
	./integration_tests.sh
clean:
	rm -f $(OBJ) $(TESTOBJ) webserver
