CXX=g++
CXXFLAGS=-std=c++11 -I. -Wall -Werror
BOOSTFLAGS = -lboost_system
DEPS=server.h connection.h config_parser.h
OBJ=server.o connection.o config_parser.o main.o

%.o: %.c $(DEPS)
	$(CC) $(CXXFLAGS) -c -o $@ $<

webserver: $(OBJ)
	g++ $(CXXFLAGS) -o $@ $^ $(BOOSTFLAGS)
	
test:
	./build_tests.sh
	./config_parser_test
	
test-curl:
	curl http://localhost:4000
clean:
	rm -f $(OBJ) webserver
