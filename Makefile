CXX=g++
CXXFLAGS=-std=c++11 -I. -Wall -Werror
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

coverage: CXXFLAGS += -fprofile-arcs -ftest-coverage
coverage: webserver
	
test-curl: 
	curl http://localhost:4000

clean:
	rm -f $(OBJ) $(TESTOBJ) webserver
