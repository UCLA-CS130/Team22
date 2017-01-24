CXX=g++
CXXFLAGS=-std=c++11 -I. -Wall -Werror
BOOSTFLAGS = -lboost_system
DEPS=server.h connection.h
OBJ=server.o connection.o main.o

%.o: %.c $(DEPS)
	$(CC) $(CXXFLAGS) -c -o $@ $<

webserver: $(OBJ)
	g++ $(CXXFLAGS) -o $@ $^ $(BOOSTFLAGS)

clean:
	rm -f $(OBJ) webserver
