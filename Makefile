CC = g++
CFLAGS = -Wall -I. -lboost_system
CXXFLAGS = -std=c++11
DEPS = server.h connection.h config_parser.h
OBJ = server.o connection.o config_parser.o main.o

%.o: %.c $(DEPS)
	$(CC) $(CXXFLAGS) -c -o $@ $< $(CFLAGS)

webserver: $(OBJ)
	g++ $(CXXFLAGS) -o $@ $^ $(CFLAGS)

test:
	./build_tests.sh
	./config_parser_test
	
test-curl:
	curl http://localhost:4000

clean:
	rm -f $(OBJ) webserver